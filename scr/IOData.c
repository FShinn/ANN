/* ***********************************************************************
 * Program: IOData.c
 * Description: Parses data from input file and creates a structure for
 *  data storage and usage. Also provides translation tools for ANN
 *  to interpret stored data.
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
#include <string.h>


struct IOData {
	char *input;
	char *output;
} IOData;

struct translation {
	int count;
	char *entries;
} translation;

int getData(struct IOData io[], char *filename, int IOCount, int inputLen, int outputLen);
int buildTranslationMatrix(int IOCount, int outputLen, struct IOData io[], struct translation translations[]);
void displayIO(int IOCount, int inputLen, int outputLen, struct IOData io[]);

// stores data in provided IOData array, parsed from file named filename
int getData(struct IOData io[], char *filename, int IOCount, int inputLen, int outputLen) {
	// malloc int* for every i,o pair
	for (int i=0; i<IOCount; i++) {
		if ((io[i].input = malloc(inputLen * sizeof(int))) == NULL) {
			fprintf(stderr, "failed to allocate memory to struct IOData io[%d].input\n", i);
			return -1;
		}
		if ((io[i].output = malloc(outputLen * sizeof(int))) == NULL) {
			fprintf(stderr, "failed to allocate memory to struct IOData io[%d].output\n", i);
			return -1;
		}
	}
	
	// open file
	FILE *readfile;
	if ((readfile = fopen(filename, "r")) == NULL) {
		fprintf(stderr,"could not open file \"%s\"\n", filename);
		return -1;
	}
	
	// prepare read buffer
	int bufLen = (inputLen+outputLen)*2+1;
	char buf[bufLen]; 
	
	// skip first line of file (column headers)
	while ((fgets(buf, bufLen, readfile) != NULL) && buf[strlen(buf)-1] != '\n');
	
	// parse each line from readfile into io[]
	for (int io_i = 0; (io_i < IOCount) && (fgets(buf, bufLen, readfile) != NULL); io_i++) {
		// store first outputLen values in io[io_i].output
		for (int out_i=0; out_i<outputLen; out_i++)
			io[io_i].output[out_i] = buf[out_i*2];
		// store next inputLen values in io[io_i].input
		for (int in_i=0; in_i<inputLen; in_i++)
			io[io_i].input[in_i] = buf[(outputLen+in_i)*2];
	}
	
	fclose(readfile);
	return 0;
}

// builds translation tables between output nodes and output characters
int buildTranslationMatrix(int IOCount, int outputLen, struct IOData io[], struct translation translations[]) {
	int entries[256]; // one entry for each possible char
	
	for (int out_i=0; out_i<outputLen; out_i++) {
		// clear entries
		for (int e_i=0; e_i<256; e_i++)
			entries[e_i] = 0;
		
		// for each io pair, flag element in entries for output
		for (int io_i=0; io_i<IOCount; io_i++)
			entries[(int)(io[io_i].output[out_i])] = 1;
		
		// count entries
		translations[out_i].count = 0;
		for (int e_i=0; e_i<256; e_i++)
			if (entries[e_i])
				translations[out_i].count++;
		
		// malloc char[] for each output node
		if ((translations[out_i].entries = malloc(sizeof(char)*translations[out_i].count)) == NULL) {
			fprintf(stderr, "failed to allocate memory to translations[%d].entries\n", out_i);
			return -1;
		}
		
		// store unique outputs in node's char[]
		int t_i = 0;
		for (int e_i=0; e_i<256; e_i++)
			if (entries[e_i])
				translations[out_i].entries[t_i++] = (char)e_i;
	}
	return 0;
}

// performs translation on data (desired) output, character -> number
double translateOutput(char c, struct translation translationSet) {
	for (int t_i=0; t_i<translationSet.count; t_i++)
		if (c == translationSet.entries[t_i])
			return (t_i + 0.5)/translationSet.count; // 0.5 targets center of range which yields index t_i
	fprintf(stderr, "character not found in translation entries\n");
	return -1;
}

// performs translation on data input, character -> number
double translateInput(char c) {
	return c/256.0;
}

void displayIO(int IOCount, int inputLen, int outputLen, struct IOData io[]) {
	for (int io_i=0; io_i<IOCount; io_i++) {
		fprintf(stdout, "IOData[%d]: input: %c",io_i, (char)io[io_i].input[0]);
		
		for (int in_i=1; in_i<inputLen; in_i++)
			fprintf(stdout, ", %c", (char)io[io_i].input[in_i]);
		
		fprintf(stdout, "  output: %c", (char)io[io_i].output[0]);
		for (int out_i=1; out_i<outputLen; out_i++)
			fprintf(stdout, ", %c", (char)io[io_i].output[out_i]);
		
		fprintf(stdout, "\n");
	}
}

void cleanupIO(struct IOData *io, int IOCount) {
	for (int io_i=0; io_i<IOCount; io_i++) {
		free(io[io_i].input);
		free(io[io_i].output);
	}
}

void cleanupTranslations(struct translation *translations, int outputLen) {
	for (int o_i=0; o_i<outputLen; o_i++)
		free(translations[o_i].entries);
}


