# bit-sudoku
Sudoku solver implemented using bitvectors.


Each cells is represented as a bitvector, where each element in the bitvector represents a possible value for that cell. As different strategies are employed, values are removed from the bitvector until one value--the solution for that cell--remains.
