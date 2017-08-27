CC = gcc
CFLAGS = -std=gnu99 -pedantic -Wall -Werror
CFLAGS += -g

STUDENT = ..

GRADERFILES = tests.o grader.o
HFILES = my_malloc.h malloc_common.h tests.h grader.h

grade: grader
	./grader
	mv graded.txt "$(STUDENT)"

# Override what the student has!
LIBRARY = malloc
export CC CFLAGS LIBRARY

$(STUDENT): 
	$(MAKE) -e -C "$(STUDENT)" libmalloc.a

grader: $(GRADERFILES) $(STUDENT)
	$(CC) $(CFLAGS) -L "$(STUDENT)" $(GRADERFILES) -lmalloc -o $@

%.o: %.c $(HFILES)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(GRADERFILES) grader

.PHONY: clean $(STUDENT)
