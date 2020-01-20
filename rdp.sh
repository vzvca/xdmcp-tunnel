#! /bin/sh
# --------------------------------------------------------------------------
#  Starting a remote desktop
# --------------------------------------------------------------------------
proc usage { msg {status 1} } {
    puts stderr "$::argv0 error: $msg"
    puts stderr "usage: $argv0 [-d display-number] user@remote"
    puts stderr "\t-d display-number"
    exit $status
}

wexec Xnest -listen tcp -from 127.0.0.1 :${display} &
wexec ssh ${remote} xdmcptunnel -d ${display}
