/*
 * CS3600, Spring 2013
 * Project 4 Starter Code
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

#include "3600sendrecv.h"

int BUFFER_SIZE = 146000;
char BUF[146000];
int RECV = 0;
header OOO[100 * sizeof(header)];
int COUNT = 0;

void reorderArray(int min) {
  int i;
  mylog("recv buf [");
  for(i = 0; i < COUNT; i++){
    mylog("%d (%d), ", OOO[i].sequence, OOO[i].length);
  }
  mylog("]\n");
  COUNT--;
  for(i = min; i < COUNT; i++) {
    OOO[i] = OOO[i+1];
  }
}

int isInBuf(unsigned int sequence) {
  int i;
  for(i = 0; i < COUNT; i++){
    if(sequence == OOO[i].sequence)
      return i;
  }
  return -1;
}

int isAlreadyReceived(unsigned int sequence) {
  int i = isInBuf(sequence);
  if(i >= 0) {
    int size = OOO[i].length;
    reorderArray(i);
    mylog("already received %d\n", sequence);
    return size;
  }
  return 0;
  /*for(i = 0; i < COUNT; i++) {
    if(sequence == OOO[i].sequence) {
      int size = OOO[i].length;
      reorderArray(i);
      mylog("already received %d\n", sequence);
      return size;
     }
   }
   return 0; */
}

int writeToBuf(char* data, int size, unsigned int sequence) {

  if(RECV + size >= BUFFER_SIZE) {
    int newSize = BUFFER_SIZE-RECV;
    memcpy(BUF+RECV, data, newSize);
    RECV = 0;
    write(1, BUF, BUFFER_SIZE);
    writeToBuf(data+newSize, size-newSize, sequence);
  }
  else {
    memcpy(BUF+RECV, data, size);
    RECV = RECV + size;
    sequence = sequence + size;
    while(1) {
      size = isAlreadyReceived(sequence);
      if(size == 0)
        break;
      RECV = RECV + size;
      sequence = sequence + size;
    }
  }
  return sequence; 
}

void writeOutOfOrder(char* data, int size, int sequence) {  
  mylog("adding %d to ooo\n", sequence);
  memcpy(BUF+(sequence%146000), data, size);
  OOO[COUNT].sequence = sequence;
  OOO[COUNT].length = size;
  COUNT++;
}

int main() {
  /**
   * I've included some basic code for opening a UDP socket in C, 
   * binding to a empheral port, printing out the port number.
   * 
   * I've also included a very simple transport protocol that simply
   * acknowledges every received packet.  It has a header, but does
   * not do any error handling (i.e., it does not have sequence 
   * numbers, timeouts, retries, a "window"). You will
   * need to fill in many of the details, but this should be enough to
   * get you started.
   */

  // first, open a UDP socket  
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // next, construct the local port
  struct sockaddr_in out;
  out.sin_family = AF_INET;
  out.sin_port = htons(0);
  out.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sock, (struct sockaddr *) &out, sizeof(out))) {
    perror("bind");
    exit(1);
  }

  struct sockaddr_in tmp;
  int len = sizeof(tmp);
  if (getsockname(sock, (struct sockaddr *) &tmp, (socklen_t *) &len)) {
    perror("getsockname");
    exit(1);
  }

  mylog("[bound] %d\n", ntohs(tmp.sin_port));

  // wait for incoming packets
  struct sockaddr_in in;
  socklen_t in_len = sizeof(in);

  // construct the socket set
  fd_set socks;

  // construct the timeout
  struct timeval t;
  t.tv_sec = 30;
  t.tv_usec = 0;

  // our receive buffer
  int buf_len = 1500;
  void* buf = malloc(buf_len);

  unsigned int prev_sequence = 0;
  header *responseheader;

  while (1) {
    header *myheader;
    char *data;
    int isOrdered = 1;
    int eof = 0;
    FD_ZERO(&socks);
    FD_SET(sock, &socks);

    // wait to receive, or for a timeout
    if (select(sock + 1, &socks, NULL, NULL, &t)) {
      int received = recvfrom(sock, buf, buf_len, 0, (struct sockaddr *) &in, (socklen_t *) &in_len);
      if (received  < 0) {
        perror("recvfrom");
        exit(1);
      }

      //dump_packet(buf, received);

      myheader = get_header(buf);
      data = get_data(buf);
     
      if (myheader->magic == MAGIC) {
        if(myheader->sequence == prev_sequence) {
          prev_sequence = writeToBuf(data, myheader->length, myheader->sequence);
          mylog("[recv data] %d (%d) %s\n", myheader->sequence, myheader->length, "ACCEPTED (in-order)");
          isOrdered = 1;
        } 
        else {
          mylog("Received packet out of order: %d vs %d\n", prev_sequence, myheader->sequence); 
          isOrdered = 0;
          if(myheader->sequence > prev_sequence && isInBuf(myheader->sequence) < 0 && !myheader->eof)
            writeOutOfOrder(data, myheader->length, myheader->sequence);
        }
      } 
      else{
        mylog("[recv corrupted packet]\n");
      }
    } 
    else {
      mylog("[error] timeout occurred\n");
      write(1, BUF, RECV);
      exit(1);
    }

    if(myheader->eof && isOrdered) {
      mylog("[recv eof]\n");
      eof = 1;
    }

    mylog("[send ack] %d\n", prev_sequence);
    responseheader = make_header(prev_sequence, 0, eof, 1, BUFFER_SIZE-RECV);
    if (sendto(sock, responseheader, sizeof(header), 0, (struct sockaddr *) &in, (socklen_t) sizeof(in)) < 0) {
      perror("sendto");
      exit(1);
    }

    if (eof) { 
      mylog("[completed]\n");
      write(1, BUF, RECV);
      exit(0);
    }
    
  }
  mylog("finished loop, writing to stdout\n");
  write(1, BUF, RECV);

  return 0;
}
