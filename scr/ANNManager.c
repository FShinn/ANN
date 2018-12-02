/* ***********************************************************************
 * Program: ANNManager.c
 * Description: Builds, trains, and tests (trail) ANN
 * Author: Samuel Shinn
 * Last Modified: 11/12/2017
 * 
 * NOTES:
 *  struct network is defined in ANN.c
 *  Training is performed on entire IO set at a time,
 *   accuracy is measured and errors are handled immediately
 *   by backpropagation and weight update.
 *  Convergence is detected by maintaining a running list of
 *   recent accuracies, and comparing the current accuracy
 *   against the oldest. If the two are sufficiently similar,
 *   it's time to stop.
 * ***********************************************************************
 */


#include <time.h>
#include "ANN.c"


void printWeights(struct network network, FILE *outputFile) {
  for (int l_i=0; l_i<network.layerCount; l_i++) {
    fprintf(outputFile, "LAYER %d", l_i);
    for (int n_i=0; n_i<network.nodeCounts[l_i]; n_i++) {
      fprintf(outputFile, "\nNODE %2d:", n_i);
      for (int w_i=0; w_i<=((l_i == 0) ? network.inputLen : network.nodeCounts[l_i-1]); w_i++) {
        fprintf(outputFile, " %s%.2f", network.weights[l_i][n_i][w_i]>0?" ":"", network.weights[l_i][n_i][w_i]);
      }
    }
    fprintf(outputFile, "\n");
  }
}


int buildNetwork(struct network *network, int inputLen, int layerCount, int nodeCounts[], struct translation translations[]) {
  // set basic info
  network->inputLen = inputLen;
  network->layerCount = layerCount;
  network->nodeCounts = nodeCounts;
  network->translations = translations;

  // build layers
  if ((network->outputs = malloc(sizeof(double **)*layerCount)) == NULL) {
    fprintf(stderr, "failed to allocate memory to struct network network->outputs\n");
    return -1;
  }
  if ((network->weights = malloc(sizeof(double **)*layerCount)) == NULL) {
    fprintf(stderr, "failed to allocate memory to struct network network->weights\n");
    return -1;
  }
  fprintf(stdout, "Network Topology: %d layers\n", network->layerCount);
  fprintf(stdout, "Length of input vector: %d\n", network->inputLen);

  // build nodes
  for (int l_i=0; l_i<layerCount; l_i++) {
    if ((network->outputs[l_i] = malloc(sizeof(double *)*nodeCounts[l_i])) == NULL) {
      fprintf(stderr, "failed to allocate memory to struct network network->outputs[%d]\n", l_i);
      return -1;
    }
    if ((network->weights[l_i] = malloc(sizeof(double *)*nodeCounts[l_i])) == NULL) {
      fprintf(stderr, "failed to allocate memory to struct network network->weights[%d]\n", l_i);
      return -1;
    }
    if (l_i==0)
			fprintf(stdout, "Node counts: %d", network->nodeCounts[l_i]);
		else
			fprintf(stdout, ", %d", network->nodeCounts[l_i]);
		if (l_i == layerCount-1)
			fprintf(stdout, "\nLength of output vector: %d\n", network->nodeCounts[l_i]);
	}

  // build weights
  for (int l_i=0; l_i<layerCount; l_i++)
    for (int n_i=0; n_i<nodeCounts[l_i]; n_i++)
      if ((network->weights[l_i][n_i] = malloc(sizeof(double)*(l_i == 0 ? inputLen+1 : nodeCounts[l_i-1]+1))) == NULL) {
        fprintf(stderr, "failed to allocate memory to struct network network->weights[%d][%d]\n", l_i, n_i);
        return -1;
      }

  // randomize weights
  srand(time(NULL));
  for (int l_i=0; l_i<layerCount; l_i++)
    for (int n_i=0; n_i<nodeCounts[l_i]; n_i++)
      for (int w_i=0; w_i<=((l_i == 0) ? inputLen : nodeCounts[l_i-1]); w_i++)
        network->weights[l_i][n_i][w_i] = (((double)rand() / (double)RAND_MAX) * 2) - 1;

  return 0;
}


int convergence(int accuracy[], int convergenceRange, int epoch) {
	return accuracy[epoch%convergenceRange] - accuracy[(epoch-1+convergenceRange)%convergenceRange];
}


int train(struct network network, struct IOData io[], int trainingIOCount, int maxEpoch, double learningRate, char *dumpFileName, int precision, int convRange) {
  int epoch = 0;
	int convergenceRange = convRange;
	int accuracy[convergenceRange];
  char output[network.nodeCounts[network.layerCount-1]]; // stores ANN output
	FILE *dumpFile = NULL;
	if ((dumpFileName) && ((dumpFile = fopen(dumpFileName, "w+")) == NULL)) {
		fprintf(stderr,"could not open file \"%s\"\n", dumpFileName);
		return -1;
	}
	
  do {
    accuracy[epoch%convergenceRange] = 0;

    for (int io_i=0; io_i < trainingIOCount; io_i++) {
      // run network forward
      runForward(&network, io[io_i].input, output);

      // evaluate result
      int correct = 1; // treat as boolean
      for (int r_i=0; r_i<network.nodeCounts[network.layerCount-1]; r_i++)
        if (io[io_i].output[r_i] != output[r_i])
          correct = 0;

      // deal with result
      if (correct)
        accuracy[epoch%convergenceRange]++;
      else {
        if (BPandWeightUpdate(&network, io[io_i].input, io[io_i].output, learningRate) < 0)
          return -1; // error
				if (dumpFile)
					printWeights(network, dumpFile);
      }
    }
		fprintf(stdout, "Epoch %3d accuracy: %4d / %d = %.2f%%\n", epoch, accuracy[epoch%convergenceRange], trainingIOCount, 100*accuracy[epoch%convergenceRange]/(double)trainingIOCount);
  } while ((++epoch < maxEpoch) && 100*precision*convergence(accuracy, convergenceRange, epoch)/trainingIOCount);
	
	if (dumpFile)
		fclose(dumpFile);
  return 0;
}

int trial(struct network network, struct IOData io[], int trialIOCount) {
  int accuracy = 0;
  char output[network.nodeCounts[network.layerCount-1]]; // stores ANN output
	
	for (int io_i=0; io_i < trialIOCount; io_i++) {
		// run network forward
		runForward(&network, io[io_i].input, output);

		// evaluate result
		int correct = 1; // treat as boolean
		for (int r_i=0; r_i<network.nodeCounts[network.layerCount-1]; r_i++)
			if (io[io_i].output[r_i] != output[r_i])
				correct = 0;

		// deal with result
		if (correct)
			accuracy++;
	}
	fprintf(stdout, "Trial accuracy: %d / %d = %.2f%%\n", accuracy, trialIOCount, 100*accuracy/(double)trialIOCount);
  return 0;
}


void cleanupNetwork(struct network *network) {
  // free weights
  for (int l_i=0; l_i<network->layerCount; l_i++)
    for (int n_i=0; n_i<network->nodeCounts[l_i]; n_i++)
      free(network->weights[l_i][n_i]);

  // free nodes
  for (int l_i=0; l_i<network->layerCount; l_i++) {
    free(network->outputs[l_i]);
    free(network->weights[l_i]);
  }

  // free layers
  free(network->outputs);
  free(network->weights);
}
