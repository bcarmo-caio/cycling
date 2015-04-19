CC=gcc
CFLAGS=-Wall \
	   -Wextra \
	   -ansi \
	   -pedantic \
	   -O2 \
	   -Wno-comment \
	   -Winit-self \
	   -Wmissing-braces \
	   -Wmissing-include-dirs \
	   -Wswitch-default \
	   -Wmaybe-uninitialized \
	   -Wfloat-equal \
	   -Wundef \
	   -Wmissing-declarations \
	   -Winline \
	   -Wshadow

LDFLAGS=-lpthread

OBJS=cycling.o \
	 cyclist.o \
	 utils.o

BIN=cycling

all: $(OBJS)
	$(CC) $(OBJS) -o $(BIN) $(LDFLAGS)

debug:CFLAGS+=-g
debug:CFLAGS+=-DDEBUG
debug: all

clean:
	rm -rf $(BIN) $(OBJS)

%.o: %.c
	@echo [CC] $< -o $@ CFLAGS
	@gcc -c $< -o $@ $(CFLAGS)

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
	rm -rf $(BIN) $(OBJS) report.aux report.log ep1-caio-leonardo ep1.tar.gz
