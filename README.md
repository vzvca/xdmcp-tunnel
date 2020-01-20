# xdmcp-tunnel

Tunneling XDMCP connection through SSH connection.

The goal of this project is to be able to use XDMCP through SSH.

This tool can be used when the machine on which you need/want to open a XDMCP remote desktop :
  * is only reachable through SSH (direct SSH connection or through hops),
  * UDP connections with the machine are not allowed.
  * you only wan't to allow XDMCP UDP traffic on 127.0.0.1


## How it works

On the machine where the remote desktop will be displayed, an X server needs to be started.
On the machine whose desktop is accessed remotely, a small C program is executed. This program handles the XDMCP UDP negocitation and arranges for the X TCP traffic to be redirectd

## How to
### XDMCP activation

### Starting a local X Server

### 
