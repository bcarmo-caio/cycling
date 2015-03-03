CC=gcc
CFLAGS=-Wall -std=c11 -pedantic -O4

main: cycling.c
	$(CC) $(CFLAGS) cycling.c -o cycling

clean:
	rm -rf cycling

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
	rm -rf cycling report.aux report.log ep1-caio-leonardo ep1.tar.gz
