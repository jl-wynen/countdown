# Countdown-Numbers
Solve the numbers game from the TV show Countdown
The goal of the game is to get a target number by adding, subtracting, multiplying, dividing six other numbers.

This program constructs trees of all possible combinations of the input numbers and keeps and displays all combinations that yield the target.
There are two implementations.
One uses shared pointers to share nodes between different iterations of the tree and automate memory management
The other uses raw pointers and explicit memory management from outside of the tree.
The second one is faster but (for simplicity) only outputs strings representing the result not the full trees.

## Usage
```
mkdir build
cd build
cmake ..
make
```
The two implementations are compiled into `numbers` and `shared-numbers`.
