all: simpleCSVsorter.c merge.c strhelper.c iohelper.c
	make clean;
	gcc -c merge.c;
	gcc -c strhelper.c;
	gcc -c iohelper.c;
	gcc -c simpleCSVsorter.c;
	gcc merge.o strhelper.o iohelper.o simpleCSVsorter.o -o simpleCSVsorter;
	make clean-obj;

clean-obj:
	rm -f merge.o;
	rm -f strhelper.o;
	rm -f iohelper.o
	rm -f simpleCSVsorter.o;
clean:
	rm -f simpleCSVsorter;
	rm -f merge.o;
	rm -f strhelper.o;
	rm -f iohelper.o;
	rm -f simpleCSVsorter.o;
