
DLIBEXT = dylib
FLAGS = -dynamiclib -DUSE_TCL_STUBS
IFLAGS = -I/Library/Frameworks/Tcl.framework/Headers
LFLAGS = -L/Library/Frameworks/Tcl.framework -ltclstub

blink: blink.c
	gcc -DUSE_HIDAPI ${FLAGS}  ${IFLAGS} blink.c  ${LFLAGS} -o blink.${DLIBEXT}  -lBlink1

clean:
	rm -rf *~ blink.${DLIBEXT}
