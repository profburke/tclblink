# -*- tcl -*-
# Tcl package index file
#
if {[package vsatisfies [package provide Tcl] 9.0-]} {
    package ifneeded blink 1.0.0 \
        [list load [file join $dir libtcl9blink1.0.0.dylib] Blink]
} else {
    package ifneeded blink 1.0.0 \
        [list load [file join $dir libblink1.0.0.dylib] Blink]
}

