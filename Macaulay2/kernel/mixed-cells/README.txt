To compile, go to src/ and run
g++ -O3 mixed-cells.cpp

Then run
time ./a.out ../demicsExamples/cyclic10.dat
to test the program.

To profile
g++ -O3 mixed-cells.cpp -pg
./a.out ../demicsExamples/cyclyc11.dat
gprof >gprof.output 



How to use etags:

run
etags *.cpp *.h
in the shell.
Then in emacs place the cursor on a variable or type name and press META "." to jump to its definition?

Mysterious symbols:

  Ainv is the inverse of a (square) _submatrix_ of A
  Ainvw = Ainv*w 
