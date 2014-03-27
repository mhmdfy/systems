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
  h.id = htons(1337);
  h.qr = 0;
  h.opcode = 0;
  h.aa = 0;
  h.tc = 0;
  h.rd = 0;
  h.rd = 1;
  h.ra = 0;
  h.z = 0;
  h.rcode = 0;
  h.qdcount = htons(1);
  h.ancount = htons(0);
  h.nscount = htons(0);
  h.arcount = htons(0);

  // initialize question
  question q;
  q.qtype = htons(FLAG);
  q.qclass = htons(1);

  // initialize request with header and question
  request r;
  r.h = h;
  r.q = q;

  // send the DNS request (and call dump_packet with your request)
  unsigned char* query = malloc(SIZE+16);
  memset(query, 0, SIZE+16);
  memcpy(query, &h, 12);
  memcpy(query+12, NAME, SIZE);
  memcpy(query+SIZE+12, &q, 4);
  dump_packet(query, SIZE+16);

  // first, open a UDP socket  
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // next, construct the destination address
  struct sockaddr_in out;
  out.sin_family = AF_INET;
  out.sin_port = htons(PORT);
  out.sin_addr.s_addr = inet_addr(SERVER);

  if (sendto(sock, query, SIZE+16, 0, &out, sizeof(out)) < 0) {
    // an error occurred
    perror("Error in sendto()\n");
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

  /*******RESPONSE*******/
  unsigned char response[65536];
  // wait to receive, or for a timeout
  if (select(sock + 1, &socks, NULL, NULL, &t)) {
    if (recvfrom(sock, response, 65536, 0, &in, &in_len) < 0) {
      // an error occured
      perror("Error in recvfrom()\n");
    }
  } else {
    // a timeout occurred
    //perror("Timeout: The response took more than 5 secs.\n");
    printf("NORESPONSE\n");
    return 1;
  }

  // Getting header from response
  header h2;
  memcpy(&h2, response, 12);
  int ancount = checkHeader(h2);

  // got question but no answer
  if(ancount == 0) {
    printf("NOTFOUND\n");
    return 1;
  }

  // answer is corrupted
  if(ancount < 0)
    return 1;

  // Getting the name for the original question
  char* oname = malloc(265);
  getName(&oname, NAME, 0);

  // Getting question from response and comparing with original
  char* qname = malloc(265);
  int qlen = getName(&qname, response, 12);
  if(strcmp(oname, qname) != 0){
    perror("received response is for a different question\n");
    exit(1);
  }
  question q2;
  memcpy(&q2, response+qlen+12, 4);

  // print out the result
  // Getting answers and printing them
  answer a;
  int i = 0;
  int alen = 0;
  int offset = qlen+16; 
  char* aname = malloc(265);
 
  for(i = 0; i < ancount; i++) {
    alen = getName(&aname, response, offset);
    memcpy(&a, response+offset+alen, 10);
    offset = offset + alen + 10;
    
    offset = printAnswer(a, response, offset);
  }
  
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
  if(strcmp(flag, "-ns") == 0)
    FLAG = 2;
  else if(strcmp(flag, "-mx") == 0)
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

  char* token = malloc(strlen(server));
  strcpy(token, server);
  token = strtok(token, ".");
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
  SERVER = (server+1); 
}

void processName(char* name) {
  int i = 0;
  int c = 0;
  int len = 0;
  SIZE = strlen(name) + 2;
  char result[SIZE];
  NAME = malloc(SIZE);

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
  memcpy(NAME, &result, SIZE);
}

int checkHeader(header h) {
  if(ntohs(h.id) != 1337){ // id has to be 1337
    printf("ERROR\tReceived different id from the one sent\n");
    return -1;
  }
  if(h.qr != 1) { // indicate response
    printf("ERROR\tReceived a question instead of a response\n");
    return -1;
  }
  if(h.aa == 1) // Authoritative
    AUTH = 1;
  else
    AUTH = 0;

  if(h.tc != 0) { // truncate
    printf("ERROR\tThe answer is truncated\n");
    return -1;
  }
  if(h.ra != 1) { // recursion
    printf("ERROR\tThis answer does not support recursion\n");
    return -1;
  }
  short rcode = ntohs(h.rcode);
  if(rcode == 1) { // many cases 
    printf("ERROR\tThe name server was unable to interpret the query\n");
    return -1;
  }
  if(rcode == 2) {
    printf("ERROR\tServer faliure\n");
    return -1;
  }
  if(rcode == 3) {
    printf("ERROR\tName Error\n");
    return -1;
  }
  if(rcode == 4) {
    printf("ERROR\tNot implemented\n");
    return -1;
  }
  if(rcode == 5) {
    printf("ERROR\tRefused\n");
    return -1;
  }

  int qdcount = ntohs(h.qdcount);
  if(qdcount != 1) {
    printf("ERROR\tInvalid number for questions\n");
    return -1;
  }
  
  int ancount = ntohs(h.ancount);
  return ancount;
}

int getName(char** name, char* response, int offset){
  int n = 0;
  int pointer = 0;
  int r = offset;
  unsigned short chars = response[r];
  r++;

  while(chars > 0){

    if(chars >= 192){
      r = ((chars & 63) << 8) | response[r];
      chars = response[r];
      r++;

      if(pointer == 0)
        pointer = n+2;
    }

    int i = 0;
    for(i = 0; i < chars; i++){
      (*name)[n] = response[r];
      r++;
      n++;
    }
    chars = response[r];
    r++;
    (*name)[n] = '.';
    n++;
  }
  n--;
  (*name)[n] = '\0';
  
  if(pointer > 0)
    return pointer;
  else
    return n+2;
}

int printAnswer(answer a, char* response, int offset) {
  char* rdata = malloc(256);
  int atype = ntohs(a.atype);
  int rdlen = ntohs(a.rdlen);

  if(atype == 1) { 
    unsigned char first = response[offset];
    unsigned char second = response[offset+1];
    unsigned char third = response[offset+2];
    unsigned char forth = response[offset+3];

    printf("IP\t%d.%d.%d.%d\t", first, second, third, forth); 
    offset = offset + rdlen;
  }
  else if(atype == 5) {
    getName(&rdata, response, offset);
    printf("CNAME\t%s\t", rdata); 
    offset = offset + rdlen;
  } 
  else if(atype == 2) {
    getName(&rdata, response, offset);
    printf("NS\t%s\t", rdata);
    offset = offset + rdlen;
  }
  else if(atype == 15) {
    int pref = response[offset+1];
    getName(&rdata, response, offset+2);
    printf("MX\t%s\t%d\t", rdata, pref); 
    offset = offset + rdlen;
  }
  else {
    printf("type: %d vs %d\n", atype, a.atype);
    perror("invalid type\n");
    exit(10);
  }

  if(AUTH)
    printf("auth\n");
  else
    printf("nonauth\n");

  return offset;
}

