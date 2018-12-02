Sam Shinn
Artificial Neural Network

Files
 executable:
  test.exe
 source:
  test.c
  parseArgs.c
  IOData.c
  ANNManager.c
  ANN.c
 data:
  mushrooms.csv
 misc:
  readme.txt (this file)
  outputFiles (directory containing output of test runs)

  
  
COMPILING TEST.C
Test.c can compile on a Cygwin64 terminal, and should be run on a Cygwin64 terminal using the command

  gcc -o test test.c
  
test.c can also by compiled on a Ubuntu 16.04.3 LTS terminal using the command
  
  gcc -o test test.c -lm
  
test.c WILL NOT compile or run via Visual C++ 2015 x86 Native Build Tools Command Prompt.



RUNNING TEST.EXE
The test.exe file included in SamuelShinnBPANN.zip was compiled using Cygwin64, and will run in Cygwin64.
In any version, there are many commands which can be used to adjust the paramaters which determine how
 the ANN runs.
The basic command to run the program is

  ./test filename
  
 where "filename" is the name of a csv file containing input/output data, e.g. "mushrooms.csv". 
The program assumes that the first line of the csv file will be a comma seperated list of column names,
 and will skip it for the purposes of reading inputs.

Other commands can also be listed after filename, in any order, listed here:

 [-o n]             can be used to change the number of outputs from the default 1 to n. this will cause 
                    the first n values read from the csv input file to be interpreted as output values, 
										and treats the remaining values as inputs. n should be an integer greater than 0.
 
 [-l n]             can be used to specify how many layers should be contained in the ANN. the default 
                    number of layers is (number of inputs)/7. n should be an  integer greater than 0. 
 
 [-n n1 n2 ... nm]  can be used in conjunction with -l to specify how many nodes appear in each layer. 
                    If there are m layers, then n1 is how many nodes will be in layer 1, n2 is how many 
                    will be in layer 2, ..., and nm is how many nodes will be in layer m. All n must be 
                    greater than 0.
 
 [-r v]             can be used to change the learning rate from the default of 0.1 to any value greater 
                    than 0.
 
 [-e n]             can be used to change the maximum epoch from the default of 1000 to n. n should be 
                    an integer greater than 0.
 
 [-t v]             can be used to change the training partition ratio from the default of 0.8 to v. 
                    this value will determine how much of the I/O data is used for training and how much 
                    is used for testing. v shoud be a decimal value between 0 and 1 (exclusive).
 
 [-d dumpFileame]   can be used to request a dump of weight values to an external file specified by 
                    dumpFileName. The program will write to dumpFilename a printout of the entire 
                    network's weights every time weights are updated. WARNING: This functionality is not 
                    optimized and will dramatically increases runtime.
 
 [-b]               can be used to request a printout to stdout of the entire network's weights once 
                    before training (immediately after random initialization) and once after training.
 
 [-p n]             can be used to request that the training algorithm use a more precise measure of 
                    convergance to determine when to stop training. n is the number of decimal points 
                    which must remain the same between comparison of the current epoch's percentage of 
                    accuracy and some other epoch's percentage of accuracy. n should be a number greater 
                    than or equal to 0, though large values are inadvisable. The default value is 2.
 
 [-c n]             can be used to request that the training algorithm use a different range to compare 
										the accuracies of epochs. The program will compare the accuracy of the current epoch 
										against the accuracy of the epoch which occured n epochs ago. n should be an integer 
										greater or equal to 2, and is 32 by default.
