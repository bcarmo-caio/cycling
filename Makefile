CC=gcc
CFLAGS=-Wall -Wextra -ansi -pedantic -O2 -Wno-comment
LDFLAGS=-lpthread

all: clean cycling.c
	$(CC) $(CFLAGS) $(LDFLAGS) cycling.c -o cycling

debug: clean cycling.c
	$(CC) $(CFLAGS) -g -DDEBUG  $(LDFLAGS) cycling.c -o cycling

clean:
	rm -rf cycling *.o

report: report.tex
	pdflatex report.tex
	rm -rf report.aux report.log

package: cycling.c Makefile README report.pdf
	mkdir ep1-caio-leonardo
	cp $^ ep1-caio-leonardo
	tar cf ep1.tar ep1-caio-leonardo
	gzip --best ep1.tar
	rm -rf ep1-caio-leonardo

purge:
	rm -rf cycling *.o report.aux report.log ep1-caio-leonardo ep1.tar.gz
