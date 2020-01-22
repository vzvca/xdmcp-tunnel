#! /bin/sh
# next line restarts using tclsh \
    exec tclsh "$0" ${1+"$@"} 

# --------------------------------------------------------------------------
#  Starting a remote desktop
# --------------------------------------------------------------------------
proc usage { msg {status 1} } {
    puts stderr "$::argv0 error: $msg"
    puts stderr "usage: $::argv0 \[-d display-number\] user@remote"
    puts stderr "\t-d display-number\tinteger (default to $::display)"
    exit $status
}

proc wexec { args } {
    set amp [lindex $args end]
    if { $amp eq "&" } {
	set args [lrange $args 0 end-1]
	exec {*}$args >@ stdout 2>@ stderr &
    } else {
	exec {*}$args >@ stdout 2>@ stderr
    }	
}

# -- default
set display 5

# -- parse arguments
set len [llength $argv]
if { ($len != 1) && ($len != 3) } {
    usage "wrong number of arguments"
}
if { $len == 3 } {
    if { [lindex $argv 0] ne "-d" } {
	usage "expecting '-d' but got '[lindex $argv 0]'" 
    }
    set display [lindex $argv 1]
    if { ![string is entier $display] } {
	usage "expecting an inteer but got '${display}'"
    }
    if { $display < 0 } {
	usage "expecting a positive number"
    }
    if { $display > 100 } {
	usage "expecting a positive number below 100"
    }
}

# -- compute port
set port [expr {6000 + $display}]
set remote [lindex $argv end]

# -- launch Xnest
wexec Xnest -listen tcp -from 127.0.0.1 :${display} &

# -- open the SSH tunnel and start xdmcptunnel on remote end
wexec ssh -R ${port}:127.0.0.1:${port} ${remote} xdmcptunnel -d ${display}
