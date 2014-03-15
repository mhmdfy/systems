/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600DNS_H__
#define __3600DNS_H__

int FLAG;
char* SERVER;
short PORT;
char* NAME;


typedef struct header_s {
  int id:16;
  int qr:1;
  int opcode:4;
  int aa:1;
  int tc:1; 
  int rd:1;
  int ra:1;
  int z:3;
  int rcode:4;
  int qdcount:16;
  int ancount:16;
  int nscount: 16;
  int arcount: 16;
} header;

typedef struct queston_s {
  char* qname;
  int qtype:16;
  int qclass:16;
} question;

typedef struct request_s {
  header h;
  question q;
} request;

void processArgs(int argc, char* argv[]);
void processFlag(char* flag);
void processServerPort(char* serverPort);
void processName(char* name);

#endif

