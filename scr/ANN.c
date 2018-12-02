/* ***********************************************************************
 * Program: ANN.c
 * Description: Maintains structure and functions of ANN, i.e.
 *  feedforward running and backpropagation and weight updates.
 * Author: Samuel Shinn
 * Last Modified: 11/12/2017
 * 
 * NOTES:
 *  Neural nodes output are determined by the sigmoid function,
 *   where the input of the sigmoid is given by a weightedSum.
 *  Details of forward running and backpropagation are provided
 *   within their respective functions.
 * ***********************************************************************
 */

#include <math.h>
#include "IOData.c"

struct network {
	int inputLen;
	int layerCount;
	int *nodeCounts;
	double **outputs;
	double ***weights;
	struct translation *translations;
} network;

/************************************** info about struct network:
 * inputLen: length of input vector
 * layerCount: number of layers (excludes in, includes out)
 * nodeCounts: number of nodes in each layer
 * outputs: output values of each node from most recent runForward(..)
 * weights: weights of each node of each layer
 * translations: stores translation info between numerical and character output
 * 
 * 
 * length of nodeCounts = layerCount
 * 
 * length of outputs = layerCount
 * length of outputs[layer_i] = nodeCounts[layer_i]
 * 
 * length of weights = layerCount
 * length of weights[layer_i] = nodeCounts[layer_i]
 * length of weights[layer_i][node_i] = nodeCounts[layer_i-1] + 1
 *    (+1 to include bias weight)
 *    except length(weights[layer_i==0][node_i]), which = inputLen + 1
 * 
 * length of translations = nodeCounts[layerCount-1] (i.e. outputLen)
 */

double sigmoid(double sum) {
	return 1.0/(1.0+exp(0.0-sum));
}

double sumDeltasNextLayer(int currentNode, double** weights, double *deltas, int count) {
	double sum = 0.0;
	for (int d_i=0; d_i<count; d_i++)
		sum += weights[d_i][currentNode+1]*deltas[d_i];
	return sum;
}



void runForward(struct network *network, char *input, char *output) {
	/* For each node in the network, 
	 * network->outputs[l_i][n_i] first temporarilly stores the weightedSum
	 * and then finally stores sigmoid(weightedSum)
	 */
	for (int l_i=0; l_i<network->layerCount; l_i++) {
		for (int n_i=0; n_i<network->nodeCounts[l_i]; n_i++) {
			for (int w_i=0; w_i<=(l_i==0?network->inputLen:network->nodeCounts[l_i-1]); w_i++) {
				if (w_i == 0)
					// resets node's weightedSum to bias value
					network->outputs[l_i][n_i] = network->weights[l_i][n_i][w_i];
				else
					// add input multiplied by weight to running weightedSum 
					// (for the first layer, input source is char *input parameter, but all other layers use as input the output of the preceeding layer)
					network->outputs[l_i][n_i] += network->weights[l_i][n_i][w_i]*(l_i==0?translateInput(input[w_i-1]):network->outputs[l_i-1][w_i-1]);
			}
			// store sigmoid(weightedSum) as node's output
			network->outputs[l_i][n_i] = sigmoid(network->outputs[l_i][n_i]);
		}
	}
	// convert output of outputLayer to corresponding char values and store as vector in char *output parameter
	for (int o_i=0; o_i<network->nodeCounts[network->layerCount-1]; o_i++)
		output[o_i] = network->translations[o_i].entries[(int)(network->translations[o_i].count*network->outputs[network->layerCount-1][o_i])];
}


int BPandWeightUpdate(struct network *network, char *input, char *desiredOutput, double learningRate) {
  // malloc delta matrix (matrix is "ragged", secondary dimension are of different lengths)
  double **delta = malloc(sizeof(double*)*(network->layerCount));
  if (delta == NULL) {
    fprintf(stderr, "failed to allocate memory to delta\n");
    return -1;
  }
  /* calculate delta values for all nodes, working backward through layers
	 * for all nodes, delta is determined by differential of sigmoid function,
	 *  i.e. nodeOutput * (1 - Output), and by then multipling by...
	 *    for last (output layer), * (desiredOutput - nodeOutput)
	 *    for all other nodes, * sumForAllNodesInNextLayer(deltaOfNodeInNextLayer*weightConnectingThisNodeToNodeInNextLayer)
	 */
  for (int l_i=network->layerCount-1; l_i>=0; l_i--) {
    if ((delta[l_i] = malloc(sizeof(double)*(network->nodeCounts[l_i]))) == NULL) {
      fprintf(stderr, "failed to allocate memory to delta[%d]\n", l_i);
      return -1;
    }
    for (int n_i=0; n_i<network->nodeCounts[l_i]; n_i++) {
      double factorOfDelta;
      // for output layer
      if (l_i == network->layerCount-1) {
        if ((factorOfDelta = translateOutput(desiredOutput[n_i], network->translations[n_i])) < 0) {
          return -1; // error
        }
        else {
          factorOfDelta -= network->outputs[l_i][n_i];
        }
      }
      // for hidden layers
      else {
        factorOfDelta = sumDeltasNextLayer(n_i, network->weights[l_i+1], delta[l_i+1], network->nodeCounts[l_i+1]);
      }
      delta[l_i][n_i] = network->outputs[l_i][n_i]*(1.0-network->outputs[l_i][n_i])*factorOfDelta;
    }
  }
  /* update weights
	 * for all weights, the change in the weight is determined by
	 *  the learningRate
	 *  the output of the node which is at the front of the weight
	 *  the delta value of the node which is at the receiving end of the weight
	 */
  for (int l_i=network->layerCount-1; l_i>=0; l_i--) {
    for (int n_i=0; n_i<network->nodeCounts[l_i]; n_i++) {
      for (int w_i=0; w_i<=((l_i == 0) ? network->inputLen : network->nodeCounts[l_i-1]); w_i++) {
        // update weight
        double in = w_i == 0? 1 : (l_i == 0? translateInput(input[w_i-1]) : network->outputs[l_i-1][w_i-1]);
        network->weights[l_i][n_i][w_i] += learningRate*in*delta[l_i][n_i];
      }
    }
		free(delta[l_i]);
  }
  free(delta);
  return 0;
}





