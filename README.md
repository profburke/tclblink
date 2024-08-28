
# Blink

ThingM's [blink(1)](https://blink1.thingm.com/) is a programmable status indicator that plugs into a USB port. This library allows you to control a blink(1) via Tcl code.

I originally developed a library in Lua [(Luablink)](https://github.com/profburke/luablink) to control blink(1)'s which is much better documented and much more mature. My intention is to catch up and keep the two libraries on par.

## Usage

If you just want a pretty light, and assuming the code is in the search path for Tcl packages, it's as simple as:

```tcl
    set d [blink open 0]
    blink set $d 255 123 147
```

This slightly more extensive example toggles the blink between red and blue:

```tcl
    proc oscillate {d reps ms} {
      for {set i 0} {$i < $reps} {incr i} {
        blink red $d
        after $ms
        blink blue $d
        after $ms
      }
      
    set d [blink open 0]
    oscillate $d 4 300
```
              
## Requirements and Installation

The library was developed using [Tcl](http://tcl.tk) 9.0b2. However it has been tested with Tcl 8.6.14 and seems to run fine. 

It does depend on the Blink1 [command line tool](https://github.com/todbot/blink1/blob/master/docs/blink1-tool.md). Currently the `Makefile` assumes this is already built and, specifically, that the include and dynamic library files are available in `/usr/local/include` and `/usr/local/lib` respectively. If you have these files in a different directory, you can adjust the `Makefile` as appropriate.

Future versions of the build process may handle downloading, building and installing the Blink1 library automatically if it's not currently available. The `Makefile` is dependent on GNU Make 4.0 or later.

In a nutshell, here's how to grab, build, and install the software:

```
git clone https://github.com/profburke/tclblink.git
cd tclblink
make 

```

You will need to move the `blink1.0.0` subdirectory (_which includes the dynamic libarary and `pkgIndex.tcl` to somewhere on the Tcl package search path.

## Documentation

Documentation is not even minimal at the moment, consisting of this README file and the source itself.

## Contributions and Contact Information

**Help and contributions are encouraged and greatly appreciated!** 

Code changes, documentation improvement, more examples, ..., a cool logo&mdash;there are a wide range of ways you can contribute. A few particular needs are: one or more people to test this software on a Linux machine, help porting to Windows, and other platforms as well.

Of particularly **note**, it's been a while since I've done much Tcl programming, so I would appreciate suggestions on how better to match Tcl idioms, fit better into standard Tcl build processes, etc.

The best way to contact me regarding this library is to post an issue to the [github repository](https://github.com/profburke/luablink/issues).

## License

Blink is free software distributed under the terms of the MIT license. It may be used for any purpose, including commercial purposes, at absolutely no cost without having to ask us. The only requirement is that if you do use blink, then you should give us credit by including the appropriate copyright notice somewhere in your product or its documentation. For details, see [LICENSE](https://github.com/profburke/luablink/blob/master/LICENSE).

