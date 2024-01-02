# bit-sudoku
Sudoku solver implemented using bitvectors.

## Concept

Each cell in the sudoku puzzle is represented as a bitvector of candidates. As different strategies are employed towards solving the puzzle, values are removed from the bitvectors until one value remains in each--the solution for that cell.

## Current Approaches

### Traditional Solver
This solver, implemented in `ts.c`, 
repeatedly applies traditional (human) sudoku strategies to the puzzle 
until it is either solved or a maximum number of iterations have been exceeded. 
This solver currently succeeds at easy, medium, and some hard puzzles. 
Future work includes optimizing when different strategies are applied, 
looking for places to increase performance, 
and implementing more strategies.

### Stack-Based Solver
The gist of this approach is to progress through the puzzle, 
choosing one candidate from each cell until the puzzle becomes impossible to solve. 
At that point, the puzzle is reverted to the last valid state and a different solution is attempted. 
This solver is implemented in `ss.c`
and successfully solves all levels of sudoku puzzles.
It is a more complex backtracking-type algorithm and uses a stack to track the transformations applied to the puzzle,
hence its name.

### Optimized Stack-Based Solver
This (`ss-opt.c`) is a version of the stack-based solver above,
currently a playground for experimenting with different optimization techniques.
Solves all puzzles and supports a batch mode.
Current bottleneck is file operation system calls.

### Backtracking Algorithm
An implementation in `bt.c` of the backtracking algorithm described [here](https://en.wikipedia.org/wiki/Sudoku_solving_algorithms#Backtracking).
In contrast to the other solvers, each cell in the grid will only ever have one bit set. 
The initial state of each vector is 1 (0th bit),
and this bit is left-shifted as it is incremented and new values are tried.

## Thanks
The majority of the test cases used in evaluating the solvers are from the [Sudoku Exchange Puzzle Bank](https://github.com/grantm/sudoku-exchange-puzzle-bank).
Several of the testing scripts are specifically designed for parsing & testing using these puzzle files.
