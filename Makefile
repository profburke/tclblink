SHELL := bash
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules
ifeq ($(origin .RECIPEPREFIX), undefined)
  $(error This Make does not support .RECIPEPREFIX. Please use GNU Make 4.0 or later)
endif
.RECIPEPREFIX = >

UNAME := $(shell uname -s)

ifeq "$(UNAME)" "Darwin"
        OS = macos
endif

ifeq "$(OS)" "Windows_NT"
        OS = windows
endif

ifeq "$(UNAME)" "Linux"
        OS = linux
endif

ifeq "$(UNAME)" "FreeBSD"
        OS = freebsd
endif

ifeq "$(UNAME)" "OpenBSD"
        OS = openbsd
endif

ifeq "$(UNAME)" "NetBSD"
       OS = netbsd
endif

VERSION = 1.0.0
DLIBEXT = so
FLAGS = -dynamiclib -DUSE_TCL_STUBS
IFLAGS = -I/Library/Frameworks/Tcl.framework/Headers
LFLAGS = -L/Library/Frameworks/Tcl.framework -ltclstub

ifeq "$(OS)" "macos"
DLIBEXT = dylib
endif

blink: blink.c
> clang -DDEBUG_PRINTF -DUSE_HIDAPI ${FLAGS} ${IFLAGS} ${LFLAGS} -o blink${VERSION}/libtcl9blink${VERSION}.${DLIBEXT} blink.c  -lBlink1
ifeq "$(OS)" "macos"
> install_name_tool -change libBlink1.dylib /usr/local/lib/libBlink1.dylib blink${VERSION}/libtcl9blink${VERSION}.dylib
endif

clean:
> rm -f *~ blink${VERSION}/blink.${DLIBEXT}
