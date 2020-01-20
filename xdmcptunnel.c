// gcc xdmcptunnel.c -o xdmcptunnel -lX11 -g3
#include <X11/Xdmcp.h>
#include <sys/select.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <endian.h>
#include <assert.h>

CARD16 display = 5;
CARD32 sessionid = -1;
char *ip = "127.0.0.1";

int udpport = XDM_UDP_PORT;
int udpsock = -1;

#define AUTHNAME0 "MIT-MAGIC-COOKIE-1"
#define AUTHNAME1 "XDM-AUTHORIZATION-1"
#define L0 (strlen(AUTHNAME0)-1)
#define L1 (strlen(AUTHNAME1)-1)

#define TIMEOUT 0xcafe

#define DBG(x) puts( #x )

void dieif(int res, char *s)
{
  if ( res == -1 ) {
    perror(s);
    exit(1);
  }
}

CARD32 getip(char *ip)
{
  struct in_addr in;
  int res = inet_aton(ip, &in);
  dieif((res==0)?-1:0, "invalid ip");
  return (CARD32) in.s_addr;
}


unsigned char *addip( unsigned char *pkt, char *val)
{
  *(CARD32*)pkt = getip(val);
  return pkt + sizeof(CARD32);
}

unsigned char *add32( unsigned char *pkt, CARD32 val)
{
  *(CARD32*)pkt = htobe32(val);
  return pkt + sizeof(CARD32);
}

unsigned char *add16( unsigned char *pkt, CARD16 val)
{
  *(CARD16*)pkt = htobe16(val);
  return pkt + sizeof(CARD16);
}

unsigned char *add8( unsigned char *pkt, CARD8 val)
{
  *(CARD8*)pkt = val;
  return pkt + sizeof(CARD8);
}

unsigned char *addxx( unsigned char *pkt, char *s, int l)
{
  memcpy(pkt, s, l);
  return pkt+l;
}

unsigned char *addst( unsigned char *pkt, char *s)
{
  int l = strlen(s);
  pkt = add16( pkt, l);
  pkt = addxx( pkt, s, l);
  return pkt;
}

unsigned char *header(unsigned char *pkt, xdmOpCode opcode, CARD16 length)
{
  unsigned char *opkt = pkt;
  pkt = add16( pkt, (CARD16) 1);
  pkt = add16( pkt, (CARD16) opcode);
  pkt = add16( pkt, (CARD16) length);
  assert(pkt-opkt == 6);
  return pkt;
}

/* Create a query packet and returns its size */
int query(unsigned char *pkt)
{
  unsigned char *opkt = pkt;
  pkt = header( pkt, QUERY, 1);
  pkt = add8( pkt, 0);
  return pkt-opkt;
}


int request(unsigned char *pkt, CARD16 display, char *ip)
{
  unsigned char *opkt = pkt;
  pkt = header(pkt, REQUEST, 60);
  pkt = add16( pkt, display);
  pkt = add8 ( pkt, 1);         // number of connection types
  pkt = add16( pkt, 0);         // type internet 
  pkt = add8 ( pkt, 1);         // number of connection address
  pkt = add16( pkt, 4);         // Length of IP address
  pkt = addip( pkt, ip);        // IP address
  pkt = add16( pkt, 0);         // length of authentication name
  pkt = add16( pkt, 0);         // length of authentication data
  pkt = add8 ( pkt, 2);         // count of authorization names
  pkt = addst( pkt, AUTHNAME0); // length of 1st authorization name
  pkt = addst( pkt, AUTHNAME1); // length of 2nd authorization name
  pkt = add16( pkt, 0);
  return pkt-opkt;
}

int manage(unsigned char *pkt, CARD32 sessionid, CARD16 display)
{
  unsigned char *opkt = pkt;
  pkt = header(pkt, MANAGE, 23);
  pkt = add32( pkt, sessionid);
  pkt = add16( pkt, display);
  pkt = addst( pkt, "MIT-unspecified");
  return pkt-opkt;
}

int refuse(unsigned char *pkt, CARD32 sessionid)
{
  unsigned char *opkt = pkt;
  pkt = header(pkt, REFUSE, 4);
  pkt = add32( pkt, sessionid );
  return pkt-opkt;
}

int failed(unsigned char *pkt, CARD32 sessionid, char *status)
{
  unsigned char *opkt = pkt;
  int len = strlen(status);
  pkt = header( pkt, FAILED, 6 + len);
  pkt = add32( pkt, sessionid );
  pkt = addst( pkt, status);
}

int alive(unsigned char *pkt, CARD32 sessionid, CARD8 alive)
{
  unsigned char *opkt = pkt;
  pkt = header(pkt, ALIVE, 5);
  pkt = add8 ( pkt, alive);
  pkt = add32( pkt, sessionid );
  return pkt-opkt;
}

int keepalive(unsigned char *pkt, CARD16 display, CARD32 sessionid)
{
  unsigned char *opkt = pkt;
  pkt = header(pkt, KEEPALIVE, 6);
  pkt = add16( pkt, display );
  pkt = add32( pkt, sessionid );
  return pkt-opkt;
}

int opcode(unsigned char *pkt)
{
  CARD16 *res = (CARD16*)pkt;
  return be16toh(res[1]);
}

int opensocket()
{
  struct sockaddr_in me;
  int res;

  res = udpsock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  dieif(res, "socket");

  memset(&me, 0, sizeof(me));
  me.sin_family = AF_INET;
  me.sin_port = 0;  // random port
  me.sin_addr.s_addr = getip(ip);
  
  res = bind(udpsock, (struct sockaddr*)&me, sizeof(me));
  dieif(res, "bind");
}

int waitfor(unsigned char *pkt, size_t sz)
{
  struct timeval tv;
  fd_set readfds;
  int res;
  
  tv.tv_sec = 30;
  tv.tv_usec = 0;
  
  FD_ZERO(&readfds);
  FD_SET(udpsock, &readfds);

  res = select(udpsock+1, &readfds, NULL, NULL, &tv);
  switch( res ) {
  case 0:
    header(pkt, htobe16(TIMEOUT), 0);
    break;
  case 1:
    res = read( udpsock, pkt, sz);
    if ( res > 0 ) break;
    // INTENTIONAL FALL-THROUGH
  case -1:
    dieif(res, "select");
  default:
    exit(1);
  }

  return res;
}

int sendpkt( unsigned char *pkt, size_t sz )
{
  struct sockaddr_in to;  
  int res;

  memset(&to, 0, sizeof(to));
  to.sin_family = AF_INET;
  to.sin_port = htons(udpport);
  to.sin_addr.s_addr = getip(ip);

  res = sendto(udpsock, pkt, sz, 0, (struct sockaddr*) &to, sizeof(to));
  dieif(res, "sendto");
  
  return res;
}

int automate( int state )
{
  unsigned char pkt[1024];

  switch (state) {
    
  case XDM_QUERY:
    DBG(XDM_QUERY);
    opensocket();
    sendpkt( pkt, query(pkt));
    state = XDM_COLLECT_QUERY;
    break;

  case XDM_COLLECT_QUERY:
    DBG(XDM_COLLECT_QUERY);
    waitfor( pkt, sizeof(pkt) );
    switch( opcode(pkt) ) {
    case WILLING:
      state = XDM_START_CONNECTION;
      break;
    case UNWILLING:
      state = XDM_OFF;
      break;
    default:
      goto unexpected;
    }
    break;

  case XDM_START_CONNECTION:
    DBG(XDM_START_CONNECTION);
    sendpkt( pkt, request(pkt, display, ip));
    state = XDM_AWAIT_REQUEST_RESPONSE;
    break;

  case XDM_AWAIT_REQUEST_RESPONSE:
    DBG(XDM_AWAIT_REQUEST_RESPONSE);
    waitfor( pkt, sizeof(pkt) );
    switch( opcode(pkt) ) {
    case ACCEPT:
      state = XDM_MANAGE;
      break;
    case DECLINE:
      state = XDM_OFF;
      break;
    default:
      goto unexpected;
    }
    break;

  case XDM_MANAGE:
    DBG(XDM_MANAGE);
    sessionid = be32toh(*(CARD32*)(pkt+6));
    sendpkt( pkt, manage( pkt, sessionid, display));
    state = XDM_AWAIT_MANAGE_RESPONSE;
    break;

  case XDM_AWAIT_MANAGE_RESPONSE:
    DBG(XDM_AWAIT_MANAGE_RESPONSE);
    waitfor( pkt, sizeof(pkt) );
    switch( opcode(pkt) ) {
    case REFUSE:
      // selon la doc il faut faire :
      // state = XDM_START_CONNECTION
      // j'ai plutot envie de quitter
      state = XDM_OFF;
      break;
    case FAILED:
      state = XDM_OFF;
      break;
    case TIMEOUT:
      // this pseudo state is used by waitfor when it does not have received any
      // packets during X seconds.
      // we assume that it means that XOpenDisplay has been done !
      state = XDM_RUN_SESSION;
      break;
    default:
      goto unexpected;
    }
    break;

  case XDM_RUN_SESSION:
    DBG(XDM_RUN_SESSION);
    waitfor( pkt, sizeof(pkt) ); // just to wait
    sendpkt( pkt, keepalive( pkt, display, sessionid));
    state = XDM_KEEPALIVE;
    break;

  case XDM_KEEPALIVE:
    DBG(XDM_KEEPALIVE);
    waitfor( pkt, sizeof(pkt) );
    switch( opcode(pkt) ) {
    case ALIVE:
      // compare sessionID
      state = XDM_RUN_SESSION;
      // ou bien reset-display
      break;
    case TIMEOUT:
      // la doc dit de faire reset display
      state = XDM_OFF;
    }
    break;

  unexpected:
    printf("An error occured... state 0x%x\n", state);
    
  case XDM_OFF:
    printf("closing connection.\n");
    exit(0);
    break;
  }

  return state;
}


int main(int argc, char **argv)
{
  int state;
  assert(sizeof(CARD8) ==1);
  assert(sizeof(CARD16)==2);
  assert(sizeof(CARD32)==4);
  for( state = XDM_QUERY; state != XDM_OFF; /* nothing */) {
    state = automate(state);
  }
  puts("done!");
  return 0;
}
