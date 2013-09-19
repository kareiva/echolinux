RELS = testgui.h
OBJS = testgui_main.o testgui_cb.o testgui.o servercode.o threadFuncs.o


CARGS = -DNodeFont=FL_TINY_SIZE -DChatFont=FL_SMALL_SIZE \
	-DInfoFont=FL_TINY_SIZE -D_REENTRANT

#CARGS = -DNodeFont=FL_SMALL_SIZE -DChatFont=FL_SMALL_SIZE \
#	-DInfoFont=FL_SMALL_SIZE

INC = -I/usr/include/X11

echogui : $(OBJS)
	cc -o echogui -L/usr/X11R6/lib -lX11 -lXpm -lforms -lm -lpthread \
	$(OBJS)

testgui_main.o : testgui_main.c $(RELS)
	cc -c -o testgui_main.o $(CARGS) $(INC) testgui_main.c

testgui_cb.o : testgui_cb.c $(RELS)
	gcc -c -o testgui_cb.o $(CARGS) $(INC) testgui_cb.c

testgui.o : testgui.c $(RELS)
	gcc -c -o testgui.o $(CARGS) $(INC) testgui.c

servercode.o : servercode.c $(RELS)
	gcc -c -o servercode.o $(CARGS) $(INC) servercode.c

threadFuncs.o : threadFuncs.c $(RELS)
	gcc -c -o threadFuncs.o $(CARGS) $(INC) threadFuncs.c

copy_defaults:
	if [ ! -d ~/.echoLinux ]; then mkdir ~/.echoLinux; fi; \
	cp -f servers.txt ~/.echoLinux  

install: 
	cp echogui /usr/local/bin/echogui

clean:
	rm -f *.o echogui
