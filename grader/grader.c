/* MALLOC AUTOGRADER (HW 11)
 *
 * Justin Michela
 * and Joshua Cranmer
 *
 * Rewritten by Nathan Braswell to split testing functions out
 * into another file and run each in its own process.
 *
 * Updated by Roi Atalla with additional tests Fall 2016/Spring 2017
 */

#include "tests.h"
#include "grader.h"

#define FILE_NAME "graded.txt"
#define GRADER_NAME "Roi Atalla"

FILE* file;

/*
 * We make an enum of the test functions for use with the
 * switch/case statement and number of functions.
 * TEST_FUNCTION_LIST should be defined as a macro in tests.h
 */
enum test_functions {
#define X(test_func) enum_val##test_func ,
TEST_FUNCTION_LIST
#undef X
    num_functions
};
/*
 * We also make an array of the test function names for printing, etc
 * TEST_FUNCTION_LIST should be defined as a macro in tests.h
 */
char* function_names[] = {
#define X(test_func) #test_func ,
TEST_FUNCTION_LIST
#undef X
};

/*
 * We add and deduct points by printing to stdout, which
 * the master process reads and parses, adding them up.
 */
void deduct_points(int points, const char *tag, const char *message) {
    fprintf(file, "SUB: (-%d) %s: %s\n", points, tag, message);
    fflush(file);
}

void add_points(int points, const char *tag, const char *message) {
    fprintf(file, "ADD: (%d) %s: %s\n", points, tag, message);
    fflush(file);
}

int main(int argc, char** argv) {
    if (argc == 1) {
        file = fopen(FILE_NAME, "w");
        if (NULL == file)
            file = stdout;
        printf("running master\n");
        int points_earned = 0;
        int points_lost = 0;
        for (int i = 0; i < num_functions; i++) {
            const int line_length = 200;
            char line[line_length];
            char run_command[line_length];
            sprintf(run_command, "./grader %d", i);
            FILE* output = popen(run_command, "r");
            int one_func_points_earned = 0;
            int one_func_points_lost = 0;
            /* look for the slave %d done message to make sure it didn't segfault */
            int slave_segfaulted = 1;
            while (fgets(line, line_length-1, output)) {
                int pts;
                int result = sscanf(line, " ADD: ( %d )", &pts);
                if (result == 1) {
                    /*printf("%s", line);*/
                    one_func_points_earned += pts;
                } else {
                    result = sscanf(line, " SUB: ( %d )", &pts);
                    if (result == 1) {
                        /* print the reason points were subtracted to stdout and output file */
                        printf("%s", line);
                        fprintf(file, "%s", line);
                        one_func_points_lost -= pts; /* pts, is negative, so we're adding positive */
                    } else {
                        int slaveno;
                        result = sscanf(line, "slave %d done", &slaveno);
                        if (result == 1)
                            slave_segfaulted = 0;
                    }
                }
            }
            pclose(output);
            /*printf("Function #%d: %s earned %d points and lost %d points.\n", i, function_names[i], one_func_points_earned, one_func_points_lost);*/
            /* add our points gained/lost for this function into our running totals */
            points_earned += one_func_points_earned;
            points_lost += one_func_points_lost;
            if (slave_segfaulted) {
                printf("Function #%d: %s SEGFAULTED/TIMED_OUT\n", i, function_names[i]);
                fprintf(file, "Function #%d: %s SEGFAULTED/TIMED_OUT\n", i, function_names[i]);
            }

        }
        /* print final message to stdout and file */
        fprintf(file, "\nScore: %d/%d points\n", points_earned, TOTAL_POINTS_POSSIBLE);
        fprintf(file, "Any difference between points lost and points gained is due to functions segfaulting/timeing out before points can be deducted,\n");
        fprintf(file, "in which case those points are still lost by not adding them.\n");
        fprintf(file, "~%s\n", GRADER_NAME);

        printf("\nScore: %d/%d points\n", points_earned, TOTAL_POINTS_POSSIBLE);
        printf("Any difference between points lost and points gained is due to functions segfaulting/timeing out before points can be deducted,\n");
        printf("in which case those points are still lost by not adding them.\n");
        printf("~%s\n", GRADER_NAME);
        /*if (points_earned != TOTAL_POINTS_POSSIBLE-points_lost) {*/
            /*printf("The difference between these should be do to that function segfaulting, and you should see a message about that function segfaulting above\n");*/
        /*}*/
        /* done with master */
        return 0;
    }
    /* this is our starting point if we're a slave */
    file = stdout;
    printf("running slave %s\n", argv[1]);

    /* run the testcase # that we were passed on the command line */
    int function_num = strtol(argv[1], NULL, 10);
    switch (function_num) {
        /* use the TEST_FUNCTION_LIST macro to list out the cases w/ extra info and timeout alarm */
#define X(test_func) case enum_val##test_func:\
        alarm(5);\
        test_func();\
        printf("%s, function #%d complete\n", #test_func, function_num);\
        break;
TEST_FUNCTION_LIST
#undef X

        default:
            printf("INVALID SLAVE NUMBER\n");
            break;
    }
    printf("slave %d done\n", function_num);
    return 0;
}

