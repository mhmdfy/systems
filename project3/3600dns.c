/*
 * CS3600, Spring 2014
 * Project 3 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "3600dns.h"

/**
 * This function will print a hex dump of the provided packet to the screen
 * to help facilitate debugging.  In your milestone and final submission, you 
 * MUST call dump_packet() with your packet right before calling sendto().  
 * You're welcome to use it at other times to help debug, but please comment those
 * out in your submissions.
 *
 * DO NOT MODIFY THIS FUNCTION
 *
 * data - The pointer to your packet buffer
 * size - The length of your packet
 */
static void dump_packet(unsigned char *data, int size) {
    unsigned char *p = data;
    unsigned char c;
    int n;
    char bytestr[4] = {0};
    char addrstr[10] = {0};
    char hexstr[ 16*3 + 5] = {0};
    char charstr[16*1 + 5] = {0};
    for(n=1;n<=size;n++) {
        if (n%16 == 1) {
            /* store address for this line */
            snprintf(addrstr, sizeof(addrstr), "%.4x",
               ((unsigned int)p-(unsigned int)data) );
        }
            
        c = *p;
        if (isprint(c) == 0) {
            c = '.';
        }

        /* store hex str (for left side) */
        snprintf(bytestr, sizeof(bytestr), "%02X ", *p);
        strncat(hexstr, bytestr, sizeof(hexstr)-strlen(hexstr)-1);

        /* store char str (for right side) */
        snprintf(bytestr, sizeof(bytestr), "%c", c);
        strncat(charstr, bytestr, sizeof(charstr)-strlen(charstr)-1);

        if(n%16 == 0) { 
            /* line completed */
            printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
            hexstr[0] = 0;
            charstr[0] = 0;
        } else if(n%8 == 0) {
            /* half line: add whitespaces */
            strncat(hexstr, "  ", sizeof(hexstr)-strlen(hexstr)-1);
            strncat(charstr, " ", sizeof(charstr)-strlen(charstr)-1);
        }
        p++; /* next byte */
    }

    if (strlen(hexstr) > 0) {
        /* print rest of buffer if not empty */
        printf("[%4.4s]   %-50.50s  %s\n", addrstr, hexstr, charstr);
    }
}

int main(int argc, char *argv[]) {
  /**
   * I've included some basic code for opening a socket in C, sending
   * a UDP packet, and then receiving a response (or timeout).  You'll 
   * need to fill in many of the details, but this should be enough to
   * get you started.
   */

  // process the arguments
  processArgs(argc, argv);

  // construct the DNS request
  // initialize header
  header h;
  h.id = 1337;
  h.qr = 0;
  h.opcode = 0;
  h.aa = 0;
  h.tc = 0;
  h.rd = 1;
  h.ra = 0;
  h.z = 0;
  h.rcode = 0;
  h.qdcount = 1;
  h.ancount = 0;
  h.nscount = 0;
  h.arcount = 0;

  // initialize question
  question q;
  q.qname = NAME;
  q.qtype = FLAG;
  q.qclass = 1;

  // initialize request with header and question
  request r;
  r.h = h;
  r.q = q;

  // send the DNS request (and call dump_packet with your request)
  unsigned char* query;
  memcpy(query, &r, sizeof(request));
  dump_packet(query, strlen(query));

  // first, open a UDP socket  
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // next, construct the destination address
  struct sockaddr_in out;
  out.sin_family = AF_INET;
  out.sin_port = htons(PORT);
  out.sin_addr.s_addr = inet_addr(SERVER);

  if (sendto(sock, query, strlen(query), 0, &out, sizeof(out)) < 0) {
    // an error occurred
    perror("");
  }

  // wait for the DNS reply (timeout: 5 seconds)
  struct sockaddr_in in;
  socklen_t in_len;

  // construct the socket set
  fd_set socks;
  FD_ZERO(&socks);
  FD_SET(sock, &socks);

  // construct the timeout
  struct timeval t;
  t.tv_sec = 5;
  t.tv_usec = 0;

  unsigned char* response;
  // wait to receive, or for a timeout
  if (select(sock + 1, &socks, NULL, NULL, &t)) {
    // TODO comeback
    if (recvfrom(sock, response, 0, 0, &in, &in_len) < 0) {
      // an error occured
    }
  } else {
    // a timeout occurred
  }

  // print out the result
  
  return 0;
}


void processArgs(int argc, char* argv[]) {
  if(argc == 4){
    processFlag(argv[1]);
    processServerPort(argv[2]);
    processName(argv[3]);
  }
  else if(argc == 3) {
    FLAG = 1;
    processServerPort(argv[1]);
    processName(argv[2]);
  }
  else {
    perror("Wrong number of arguments\nFormat is: ./3600dns [-ns|-mx] @<server:port> <name>\n");
    exit(1);
  } 
}

void processFlag(char* flag) {
  if(strcmp(flag, "-ns"))
    FLAG = 2;
  else if(strcmp(flag, "-mx"))
    FLAG = 15;
  else {
    perror("Invalid flag: should be [-ns|-mx]\n");
    exit(1);
  }
}

void processServerPort(char* serverPort) {
  int i;
  char* server = strtok(serverPort, ":");
  char* port = strtok(NULL, ":");

  if(port != NULL)
    PORT = (short) atoi(port);
  else
    PORT = (short) 53;

  char* token = strtok(server, ".");
  token++;
  if(atoi(token) < 0 || atoi(token) > 255){
    perror("Invalid server format: should be a.b.c.d\n");
    exit(1);
  }
  for(i = 0; i < 3; i++) {
    token = strtok(NULL, ".");
    if(atoi(token) < 0 || atoi(token) > 255){
      perror("Invalid server format: should be a.b.c.d where a,b,c and d are between 0-255\n");
      exit(1);
    }
  }
  SERVER = server; 
}

void processName(char* name) {
  char result[256];
  int i = 0;
  int c = 0;
  int len = 0;

  char* token = strtok(name, ".");
  while(token != NULL) {
    len = strlen(token);
    result[c] = len;
    c++;
    for(i = 0; i < len; i++) {
      result[c] = token[i];
      c++;
    }
    token = strtok(NULL, ".");
  }
  result[c] = 0;
  result[c+1] = '\0';
  NAME = result;
}
