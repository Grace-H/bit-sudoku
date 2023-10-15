# bit-sudoku
Sudoku solver implemented using bitvectors.

## Concept

Each cell in the sudoku puzzle is represented as a bitvector of candidates. As different strategies are employed towards solving the puzzle, values are removed from the bitvectors until one value remains in each--the solution for that cell.

## Current Approaches

### Traditional Solver
This solver, implemented in `sudoku.c`, 
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
This solver is implemented in `stack-solver.c`
and successfully solves all levels of sudoku puzzles.
It uses a stack to track the transformations performed on the puzzle,
hence its name.

## Thanks
The majority of the test cases used in evaluating the solvers are from the [Sudoku Exchange Puzzle Bank](https://github.com/grantm/sudoku-exchange-puzzle-bank).
Several of the testing scripts are specifically designed for parsing & testing using these puzzle files.
