CC=gcc
#DO NOT ADD -ansi. -ansi DO NOT SUPPORT pthreads_barrier_*
CFLAGS=-Wall \
	   -Wextra \
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

LDFLAGS=-lpthread \
		-lrt

OBJS=src/cycling.o \
	 src/cyclist.o \
	 src/utils.o

BIN=cycling

all: clean $(OBJS)
	$(CC) $(OBJS) -o $(BIN) $(LDFLAGS)

debug: CFLAGS+=-g
debug: CFLAGS+=-DDEBUG
debug: clean all

clean:
	rm -rf $(BIN) $(OBJS)

src/%.o: src/%.c
	@echo [CC] $< -o $@ CFLAGS
	@gcc -c $< -o $@ $(CFLAGS)

report: report.tex
	pdflatex report.tex
	rm -rf report.aux report.log

package: src Makefile README report.pdf
	mkdir ep1-caio-leonardo
	cp -a $^ ep1-caio-leonardo
	tar cf ep1.tar ep1-caio-leonardo
	gzip --best ep1.tar
	rm -rf ep1-caio-leonardo

purge: clean
	rm -rf report.aux report.log report.pdf ep1-caio-leonardo ep1.tar.gz
