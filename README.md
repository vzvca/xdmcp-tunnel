# xdmcptunnel

## Introduction

The goal of this project is to be able to use XDMCP through SSH or more generally through a TCP connection. Tunneling RDP or VNC is easy because these protocols only rely on a single TCP connection between the client and the server. On the contrary the XDMCP protocol requires UDP which can't be tunneled through a TCP connection. The simple tool `xdmcptunnel' adds the missing parts and handles the UDP stuff of XDMCP protocol.

### When to use it

This tool can be used when it is not possible to open a direct UDP connection between the XDMCP client and the target machine. 


### How it works

On the machine where the remote desktop will be displayed, an X server needs to be started.
````
````

On the machine whose desktop is accessed remotely, a small C program is executed. This program handles the XDMCP UDP negocitation and arranges for the X TCP traffic to be tunneled back to the client machine.
````
ssh -L6005:127.0.0.1:6005 user@remote xdmcptunnel -s 5 
````

### Enabling XDMCP

XDMP is considered unsecure and is disabled by default. It is enabled by tuning the configuration of the display manager used.

#### lightdm

XDMCP is enabled by editing the file /etc/lightdm/lightdm.conf, there should be a block named XDMCPServer in the file, looking like this :
````
#
# XDMCP Server configuration
#
# enabled = True if XDMCP connections should be allowed
# port = UDP/IP port to listen for connections on
# listen-address = Host/address to listen for XDMCP connections (use all addresses if not present)
# key = Authentication key to use for XDM-AUTHENTICATION-1 or blank to not use authentication (stored in keys.conf)
# hostname = Hostname to report to XDMCP clients (defaults to system hostname if unset)
#
# The authentication key is a 56 bit DES key specified in hex as 0xnnnnnnnnnnnnnn.  Alternatively
# it can be a word and the first 7 characters are used as the key.
#
[XDMCPServer]
#enabled=false
#port=177
#listen-address=
#key=
#hostname=
````

To enable XDMCP change the block to :
````
[XDMCPServer]
enabled=false
port=177
listen-address=127.0.0.1
#key=
#hostname=
````


