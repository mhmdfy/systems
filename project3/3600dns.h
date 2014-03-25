/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600DNS_H__
#define __3600DNS_H__

unsigned int SIZE;
unsigned int FLAG;
unsigned char* SERVER;
short PORT;
unsigned char* NAME;
unsigned char* ANSWER_NAME;
int NONAUTH;

typedef struct header_s {
  unsigned int id:16;
  unsigned int rd:1; // backward
  unsigned int tc:1;
  unsigned int aa:1;
  unsigned int opcode:4;
  unsigned int qr:1; // 8
  unsigned int rcode:4; // backward
  unsigned int z:3;
  unsigned int ra:1; // 8
  unsigned int qdcount:16;
  unsigned int ancount:16;
  unsigned int nscount: 16;
  unsigned int arcount: 16;
} header;

typedef struct queston_s {
//  unsigned char* qname;
  unsigned int qtype:16;
  unsigned int qclass:16;
} question;

typedef struct answer_s {
//  unsigned char* aname;
  unsigned int atype:16;
  unsigned int aclass:16;
  unsigned int ttl:32;
  unsigned int rdlen:16;
} answer;

typedef struct request_s {
  header h;
  question q;
} request;

void processArgs(int argc, char* argv[]);
void processFlag(char* flag);
void processServerPort(char* serverPort);
void processName(char* name);
void checkHeader(header h);

#endif

