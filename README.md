# ANN

A artificial nueral network (ANN) implementation.
View [readme.txt](readme.txt) for details.

## About / Discussion

After several attempts at implementing the backpropagation algorithm, and persistently running 
into problems which required me to backtrack and rethink things, I at one point decided to 
scrap what I had been working on and aim instead for a more comprehensive, flexible, and 
adaptive approach. What results is a program which builds a neural network in whatever form the 
user requests, rather than in the form 'hard-coded' into it.

Therefore, offering a discussion on topology, learning speed, etc. is almost a moot point in 
this situation. Because the topology is user defined at the execution of the program. Even 
without user input, the topology changes depending on the data file on which it operates. 
Using the accompanying mushrooms.csv file, the default topology... 
 contains 3 layers, which contain 22, 11, and 1 node respectively. 
 In the first layer, each of the 22 nodes has 23 weights: 
  One for each of the inputs plus one for a bias value. 
 In the second layer, each of the 11 nodes also has 23 weights, 
  one for each node in the previous layer plus one for bias value. 
 In the last layer, the one and only node has 12 weights, 
  (again) enough for the output of the previous layer's nodes plus one for a bias value. 
This final layer outputs a value which will be compared against a desired value to determine 
correctness. 
The default learning speed is 0.1, a multiplier involved every time there is a weight update.
 However, this is also customizable. It does not change at runtime, however, which might be 
 something for me to consider adding in the future.
Weights are initialized to a random value between -1 and 1. This is due to the fact that the 
 sigmoid function tends to work better when the input value is small. For this same reason, the 
 input values are normalized so that they are small values (between 0 and 1) by dividing their 
 ASCII value by the maximum character value 256.

As for speed, (in testing with mushrooms.csv) the ANN trains rather quickly (assuming it isn't 
dumping weights into some file) with networks which contain 3 or fewer layers. More than that, 
however, things seem to bog down, even with high learning rates. Lowering the number of layers 
to 1 inheritly removes the ANN's capability to model nonlinear functions, but in the case of 
mushrooms.csv, this doesn't seem to be a problem. In fact, using 1 layer frequently yielded 
better accuracy than using 2 or 3 layers, and training time was essentially trivial. The only 
downside is that it makes training the ANN somewhat volitile, and will with some frequency 
prematurely stop training by detecting a false positive of convergance.

Although the program will approach 100% accuracy on the training set if asked to and given 
enough time, it seemed that 90-95% was often a better goal in every regard. Obviously it is 
quite a lot faster, but this target also does not overfit the training data. When running on 
the test data, ANN's trained to 90-95% often performed better than ANN's trained to 95-100%.

## Author

* **Sam (Forrie) Shinn** - *Sole Contributor* - [FShinn](https://github.com/FShinn)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
