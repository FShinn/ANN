/* ***********************************************************************
 * Program: parseArgs.c
 * Description: Fills a struct paramaters with values which determine
 *  ANN structure and I/O options for program execution
 * Author: Samuel Shinn
 * Last Modified: 11/12/2017
 * 
 * NOTES:
 *  The function parseArgs(..) is the function called by other files.
 *  The structure of this file is generally that parseArgs(..),
 *   for each of the fields in struct paramater, calls a corresponding
 *   function to determine the field's value.
 *  The functions each search argv for a flag and return the corresponding
 *   input value if supplied, or indicate an error, or return a default 
 *   value if no flag is supplied.
 *  The field char *filename does not need a flag and will raise an error
 *   if it is not detected.
 * ***********************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


struct paramaters {
	char *filename;
	int IOCount;
	int inputLen;
	int outputLen;
	int layerCount;
	int *nodeCounts;
	double learningRate;
	int maxEpoch;
	double trainingPartion;
	char *dumpFile;
	int PrePost;
	int precision;
	int converganceRange;
} paramaters;


char *getFileName(int argc, char** argv);
int getOutputCount(int argc, char** argv);
int getInputInfo(char *filename, int *IOCount, int *inputLen);
int getLayerCount(int argc, char** argv, int inputLen);
int getNodeCounts(int argc, char **argv, int layerCount, int nodeCounts[], int inputLen, int outputLen);
double getLearningRate(int argc, char** argv);
int getMaxEpoch(int argc, char** argv);
double getTrainingPartion(int argc, char** argv);
char *getDumpWeights(int argc, char** argv);
int getPrePostWeights(int argc, char** argv);
int getPrecision(int argc, char** argv);
int getconverganceRange(int argc, char** argv);

int findFlagArg(int argc, char** argv, char c);
void avgBetween(int arr[], int s, int e);

void printParams(struct paramaters params);



int parseArgs(int argc, char** argv, struct paramaters *params) {
	// set paramaters, use args if supplied
	if ((params->filename  = getFileName(argc, argv)) == argv[0])
		return -1;
	
	if ((params->outputLen = getOutputCount(argc, argv)) < 0)
		return -1;
	params->IOCount = 0;
	params->inputLen = 1-params->outputLen;
	if (getInputInfo(params->filename, &(params->IOCount), &(params->inputLen)) < 0)
		return -1;
	
	if ((params->layerCount = getLayerCount(argc, argv, params->inputLen)) < 0)
		return -1;
	
	if ((params->nodeCounts = malloc(params->layerCount*sizeof(int))) == NULL) {
		fprintf(stderr, "failed to allocate memory to struct paramaters params->nodeCounts\n");
		return -1;
	}
	if (getNodeCounts(argc, argv, params->layerCount, params->nodeCounts, params->inputLen, params->outputLen) < 0)
		return -1;
	
	if ((params->learningRate = getLearningRate(argc, argv)) < 0)
		return -1;
	
	if ((params->maxEpoch = getMaxEpoch(argc, argv)) < 0)
		return -1;
	
	if ((params->trainingPartion = getTrainingPartion(argc, argv)) < 0)
		return -1;
	
	if ((params->dumpFile = getDumpWeights(argc, argv)) == argv[0])
		return -1;
	
	params->PrePost = getPrePostWeights(argc, argv);
	
	if ((params->precision = getPrecision(argc, argv)) < 0)
		return -1;
	
	if ((params->converganceRange = getconverganceRange(argc, argv)) < 0)
		return -1;
	
	return 0;
}

// retrieves name of csv IO file
char *getFileName(int argc, char** argv) {
	for (int i=1; i<argc; i++) {
		if (argv[i][0] == '-')
			i++; // skip next arg if arg[i] is a flag
		else
			return argv[i]; // return address of filename arg
	}
	fprintf(stderr, "usage: test IOdataFile\nview readme.txt for paramater flags\n");
	return argv[0];
}

// if user wants to use ANN with more than one output node
int getOutputCount(int argc, char** argv) {
	int index;
	int outputLen;
	if ((index = findFlagArg(argc, argv, 'o')+1) < argc) {
		if ((outputLen = atoi(argv[index])) > 0) {
			return outputLen;
		}
		else {
			fprintf(stderr, "length of final output vector must be greater than 0\n");
			return -1; // error, entered value < 1
		}
	}
	else
		return 1; // default length of output vector
}

// reads input file to determine inputLen, inputLen, and number of entries
int getInputInfo(char *filename, int *IOCount, int *inputLen) {
	FILE *readfile;
	if ((readfile = fopen(filename, "r")) == NULL) {
		fprintf(stderr,"could not open file \"%s\"\n", filename);
		return -1;
	}
	char c;
	// assume first line is colum titles
	while (((c = fgetc(readfile)) != EOF) && (c != '\n'))
		if (c == ',')
			(*inputLen)++;
	if (*inputLen < 1) {
		fprintf(stderr,"requested outputLen must allow for inputLen of at least 1\n");
		return -1;
	}
	// count lines after titles
	while ((c = fgetc(readfile)) != EOF)
		if (c == '\n')
			(*IOCount)++;
	fclose(readfile);
	return 0; // no problems
}

// gets number of layers to build ANN with
int getLayerCount(int argc, char** argv, int inputLen) {
	int index;
	int layerCount;
	if ((index = findFlagArg(argc, argv, 'l')+1) < argc) {
		if ((layerCount = atoi(argv[index])) > 0)
			return layerCount;
		else {
			fprintf(stderr, "number of layers must be greater than 0\n");
			return -1; // error, entered value < 1
		}
	}
	else
		return inputLen / 7; // default length of output vector
}

// user may request how many nodes are in each layer by listing numbers in order of layer
int getNodeCounts(int argc, char **argv, int layerCount, int nodeCounts[], int inputLen, int outputLen) {
	int arg_i;
	if ((arg_i = findFlagArg(argc, argv, 'n')+1) < argc) {
		// have -n flag
		if (findFlagArg(argc, argv, 'l') == argc) {
			fprintf(stderr, "flag -n should not be used without specifying layerCount via flag -l\n");
			return -1; // error
		}
		int layer_i = 0;
		while (arg_i < argc && layer_i < layerCount) {
			if ((nodeCounts[layer_i++] = atoi(argv[arg_i++])) < 1) {
				fprintf(stderr, "no layer may contain less than 1 node\n");
				return -1; // error
			}
		}
		if (layer_i != layerCount) {
			fprintf(stderr, "number of nodeCounts must match number of layers\n");
			return -1; // error
		}
		if (nodeCounts[layerCount-1] != outputLen) {
			fprintf(stderr, "number of nodes in output layer (final layer) must match outputLen\n");
			return -1; // error
		}
	}
	else {
		// automatically generate default nodeCounts depending on inputLen, outputLen, and layerCount
		nodeCounts[0] = inputLen;
		nodeCounts[layerCount-1] = outputLen;
		// recursively finds a smooth transition between inputLen and outputLen
		int s=0, e=layerCount-1;
		avgBetween(nodeCounts, s, e);
	}
	return 0; // no problems
}

// get value for learningRate
double getLearningRate(int argc, char** argv) {
	int index;
	double rate;
	if ((index = findFlagArg(argc, argv, 'r')+1) < argc) {
		if ((rate = atof(argv[index])) > 0)
			return rate;
		else {
			fprintf(stderr, "learningRate must be greater than 0\n");
			return -1; // error, entered value < 0
		}
	}
	else
		return 0.1; // default learningRate
}

// get value for maxEpoch
int getMaxEpoch(int argc, char** argv) {
	int index;
	int maxEpoch;
	if ((index = findFlagArg(argc, argv, 'e')+1) < argc) {
		if ((maxEpoch = atoi(argv[index])) > 0)
			return maxEpoch;
		else {
			fprintf(stderr, "maxEpoch must be greater than 0\n");
			return -1; // error, entered value < 0
		}
	}
	else
		return 1000; // default maxEpoch
}

// get ratio of IO data which will be used for training
double getTrainingPartion(int argc, char** argv) {
	int index;
	double ratio;
	if ((index = findFlagArg(argc, argv, 't')+1) < argc) {
		if (((ratio = atof(argv[index])) > 0) && (ratio < 1))
			return ratio;
		else {
			fprintf(stderr, "trainingPartionRatio must be between 0 and 1 (exclusive)\n");
			return -1; // error, entered value < 0 or >1
		}
	}
	else
		return 0.80 ; // default trainingPartion
}

// requests program to print weight updates to specified dumpFile while training
char *getDumpWeights(int argc, char** argv) {
	int index;
	if ((index = findFlagArg(argc, argv, 'd')+1) < argc)
		return argv[index]; // return address of name of file in which to dump weights while training
	return NULL;
}

// requests program to print before and after training weights of network to stdout
int getPrePostWeights(int argc, char** argv) {
	if (findFlagArg(argc, argv, 'b') < argc)
		return 1; // flag detected
	return 0; // no flag
}

// gets number of decimal points of % accuracy which must remain the same to converge
int getPrecision(int argc, char** argv) {
	int index, ret;
	int precision = 1;
	if ((index = findFlagArg(argc, argv, 'p')+1) < argc) {
		if ((ret = atof(argv[index])) >= 0) {
			for (int i=0; i<ret; i++)
				precision *= 10;
			return precision;
		}
		else {
			fprintf(stderr, "convergance precision must be at least 0\n");
			return -1; // error, entered value < 0
		}
	}
	else
		return 100 ; // default precision
}

// gets number of epochs that must pass with little change to converge
int getconverganceRange(int argc, char** argv) {
	int index;
	int range = 1;
	if ((index = findFlagArg(argc, argv, 'c')+1) < argc) {
		if ((range = atof(argv[index])) > 1)
			return range;
		else {
			fprintf(stderr, "convergance range must be greater than 1\n");
			return -1; // error, entered value <= 1
		}
	}
	else
		return 32 ; // default range
}


// finds the index of argument containing flag c
int findFlagArg(int argc, char** argv, char c) {
	for (int i=1; i<argc; i++)
		if (argv[i][0] == '-' && argv[i][1] && argv[i][1] == c)
			return i;
	return argc; // search found no flag c
}

// used for generating a default nodeCount if not user supplied
void avgBetween(int arr[], int s, int e) {
	if (s+1 >= e)
		return;
	int m = (s+e)/2;
	arr[m]=(arr[s]+arr[e])/2;
	avgBetween(arr, s, m);
	avgBetween(arr, m, e);
}

void printParams(struct paramaters params) {
	fprintf(stdout, "filename: %s\n", params.filename);
	fprintf(stdout, "learningRate: %f   trainingPartion: %f\n", params.learningRate, params.trainingPartion);
	fprintf(stdout, "maxEpoch: %d   convergancePrecision: %d   converganceRange: %d\n", params.maxEpoch, (int)(log(params.precision)/log(10)), params.converganceRange);
	if (params.dumpFile)
		fprintf(stdout, "dumpFileName: %s\n", params.dumpFile);
}

void cleanupParams(struct paramaters *params) {
	free(params->nodeCounts);
}





