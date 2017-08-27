CS2110-malloc-grader
====================

The autograder for the malloc assignment in CS2110.

Currently, the GNUMakefile should be edited to point to the student-to-grade's folder with the submission inside.
Right now it's set to just ..
so, you can just copy this folder into their submission folder and run make. The results will both be printed out and put in a graded.txt

You should also edit grader.c to put your own name in for GRADER_NAME.

This grader runs tests in separate processes. We do this so that student's code and segfaults cannot possibly corrupt the main grading thread.
To run a paticular test, the main grading process popen's itself with a numeric argument (./grader 2) which runs that test and prints the results on standard out.
The main grading process reads the stdout of its children processes and adds up the points gained.

It will print some information to stdout and write the results to "graded.txt", or whatever name you put in as FILE_NAME in grader.c.

If you want to add additional test cases, you can just edit the tests.c/h files to add additional functions. Note in test.h how to add it to the macro list.


Debugging
---------
If you want to debug a specific test, make sure to disable in grader.c: `alarm(5);` by surrounding it in a block comment: `/*alarm(5);*/`
Then you can open the grader in GDB: `gdb grader`, then enter `run N` where N is the test number you want to step through.
