/* ***********************************************************************
 * Program: test.c
 * Description: Framework for building, training, and testing an
 *      artificial neural network (ANN) using a csv file
 * Author: Samuel Shinn
 * Last Modified: 11/12/2017
 * 
 * NOTES:
 *  In this file there is only the main function since almost all data 
 *   in main is relevant at throughout the program's entire execution
 *   and each step is uniquely executed once.
 *  It looks messier than it is due to error checking,
 *   but underneath that, functions generally do what they are named after.
 *  In reading the code for this program, I recommend viewing files as
 *   their functions are called. i.e.
 *    parseArgs.c
 *    IOData.c
 *    ANNManager.c
 *    ANN.c
 * ***********************************************************************
 */


#include "parseArgs.c"
#include "ANNManager.c"


int main(int argc, char** argv) {
	// set paramaters, use args if supplied
	struct paramaters params;
	if (parseArgs(argc, argv, &params) < 0)
		return 0; // error, quit program
	printParams(params);
	
	// build IO array
	struct IOData io[params.IOCount];
	if (getData(io, params.filename, params.IOCount, params.inputLen, params.outputLen) < 0)
		return 0; // error, quit program
	// TODO shuffle IO data ?
	
	// build translation matrix (translates ANN output to character output)
	struct translation translations[params.outputLen];
	if (buildTranslationMatrix(params.IOCount, params.outputLen, io, translations) < 0)
		return 0;
	
  fprintf(stdout, "\nBuilding ANN...\n");
	// build ANN
	struct network network;
	if (buildNetwork(&network, params.inputLen, params.layerCount, params.nodeCounts, translations) < 0)
		return 0;
	
	// "Pre" of the "Pre/Post" training weight printout
	if (params.PrePost) {
		fprintf(stdout, "\nPre training weights:\n");
		printWeights(network, stdout);
	}
	
	// train ANN
  fprintf(stdout, "\nTraining ANN...\n");
	// CPU timing
	clock_t start, end;
	start = clock();
	if (train(network, io, (int)(params.IOCount*params.trainingPartion), params.maxEpoch, params.learningRate, params.dumpFile, params.precision, params.converganceRange) < 0)
		return 0;
	end = clock();
	double elapsedTime = ((double) (end - start)) / CLOCKS_PER_SEC;
	
	// "Post" of the "Pre/Post" training weight printout
	if (params.PrePost) {
		fprintf(stdout, "\nPost training weights:\n");
		printWeights(network, stdout);
	}
	
  fprintf(stdout, "\nTesting ANN...\n");
	// test ANN
	if (trial(network, io+(int)(params.IOCount*params.trainingPartion), params.IOCount-(int)(params.IOCount*params.trainingPartion)) < 0)
		return 0;
	fprintf(stdout, "CPU time spent training: %.2fs\n", elapsedTime);
	
	// free allocated memory
	cleanupNetwork(&network);
	cleanupTranslations(translations, params.outputLen);
	cleanupIO(io, params.IOCount);
	cleanupParams(&params);
	return 0;
}




