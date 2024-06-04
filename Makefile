
FLAGS = -dynamiclib -DUSE_TCL_STUBS
IFLAGS = -I/Library/Frameworks/Tcl.framework
LFLAGS = -L/Library/Frameworks/Tcl.framework -ltclstub

blink: blink.c
	gcc -DUSE_HIDAPI ${FLAGS}  ${IFLAGS} blink.c  ${LFLAGS} -o blink.dylib -lBlink1

hello: hello.c
	gcc ${FLAGS}  ${IFLAGS} hello.c  ${LFLAGS} -o hello.dylib

random: random.c
	gcc ${FLAGS}  ${IFLAGS} random.c  ${LFLAGS} -o random.dylib
