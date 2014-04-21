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

static int DATA_SIZE = 1460;
int BUFFER_SIZE = 146000;
char BUF[146000];
int SENTACKED = 0;
int SENT = 0;
int FIN = 0;

void usage() {
  printf("Usage: 3600send host:port\n");
  exit(1);
}

int verify(int sequence) {
  if(SENTACKED < SENT) {
    int newSentAck = SENTACKED+sequence;
    if(newSentAck > SENT)
      return 0;
    else{
      SENTACKED = newSentAck;
      return 1;
    }
  }
  else if(SENTACKED > SENT) {
    int newSentAck = SENTACKED+sequence;
    if(newSentAck < BUFFER_SIZE) {
      SENTACKED = newSentAck;
      return 1;
    }
    else {
      newSentAck = newSentAck % BUFFER_SIZE;
      if(newSentAck > SENT)
        return 0;
      else {
        SENTACKED = newSentAck;
        return 1;
      }
    }
  }
  else
    return 0;  
}

//reads from stdin and puts data into buffer
int readin(int size){
  int len = 0;
  if(FIN < SENTACKED){
    if(FIN+size > SENTACKED)
      size = SENTACKED-FIN;
    len = read(0, BUF+FIN, size);
    FIN = FIN + len;
  }
  else{
    if(FIN+size > BUFFER_SIZE)
      size = BUFFER_SIZE-FIN;
    len = read(0, BUF+FIN, size);
    FIN = FIN + len;
    if(FIN == BUFFER_SIZE)
      FIN = 0;
  }
  return len;
}

/**
 * Reads the next block of data from the buffer
 */
int get_next_data(int sequence, char *data, int size) {
  if(SENT != (sequence % BUFFER_SIZE))
    SENT = sequence % BUFFER_SIZE;

  if(FIN > SENT){
    if(FIN-SENT < size)
      size = FIN-SENT;
  }
  else if (FIN < SENT){
    if(BUFFER_SIZE-SENT < size)
      size = BUFFER_SIZE-SENT;
  }
  else {
    return 0;
  }
  memcpy(data, BUF+SENT, size);
  SENT = SENT+size;
  return size;
}

/**
 * Builds and returns the next packet, or NULL
 * if no more data is available.
 */
void *get_next_packet(int sequence, int *len) {
  char *data = malloc(DATA_SIZE);
  int data_len = get_next_data(sequence, data, DATA_SIZE);

  if (data_len == 0) {
    free(data);
    return NULL;
  }

  header *myheader = make_header(sequence, data_len, 0, 0, 1);
  void *packet = malloc(sizeof(header) + data_len);
  memcpy(packet, myheader, sizeof(header));
  memcpy(((char *) packet) +sizeof(header), data, data_len);

  free(data);
  free(myheader);

  *len = sizeof(header) + data_len;

  return packet;
}

int send_next_packet(int sequence, int sock, struct sockaddr_in out) {
  int packet_len = 0;
  void *packet = get_next_packet(sequence, &packet_len);

  if (packet == NULL) 
    return 0;

  mylog("[send data] %d (%d)\n", sequence, packet_len - sizeof(header));

  if (sendto(sock, packet, packet_len, 0, (struct sockaddr *) &out, (socklen_t) sizeof(out)) < 0) {
    perror("sendto");
    exit(1);
  }

  return packet_len - sizeof(header);
}

void send_final_packet(int sequence, int sock, struct sockaddr_in out) {
  header *myheader = make_header(sequence, 0, 1, 0, 1);
  mylog("[send eof]\n");

  if (sendto(sock, myheader, sizeof(header), 0, (struct sockaddr *) &out, (socklen_t) sizeof(out)) < 0) {
    perror("sendto");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
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

  // extract the host IP and port
  if ((argc != 2) || (strstr(argv[1], ":") == NULL)) {
    usage();
  }

  char *tmp = (char *) malloc(strlen(argv[1])+1);
  strcpy(tmp, argv[1]);

  char *ip_s = strtok(tmp, ":");
  char *port_s = strtok(NULL, ":");
 
  // first, open a UDP socket  
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // next, construct the local port
  struct sockaddr_in out;
  out.sin_family = AF_INET;
  out.sin_port = htons(atoi(port_s));
  out.sin_addr.s_addr = inet_addr(ip_s);

  // socket for received packets
  struct sockaddr_in in;
  socklen_t in_len = sizeof(in);

  // construct the socket set
  fd_set socks;

  // construct the timeout
  struct timeval t;
  t.tv_sec = 25;
  t.tv_usec = 0;

  struct timeval ack_t;
  ack_t.tv_sec = 2;
  ack_t.tv_usec = 0;

  unsigned int sequence = 0;
  //readin(DATA_SIZE*2);

  int window = 1;
  int slow_start = 100;

  while(1) {
    readin(BUFFER_SIZE/2);

    int i;
    int j;
    int isSent = 0;
    int temp_seq = sequence;

    for(i = 0; i < window; i++) {
      int packet_len = send_next_packet(temp_seq, sock, out);
      if(packet_len > 0) {
        isSent = 1;
        temp_seq = temp_seq + packet_len;
      }
      else{
        break;
      }
    }

    if(!isSent)
      break;
    FD_ZERO(&socks);
    FD_SET(sock, &socks);
   // int done = 0;
    if (select(sock +1, &socks, NULL, NULL, &t)){

    //while (! done) {
    for(j = 0; j < i; j++) {
      FD_ZERO(&socks);
      FD_SET(sock, &socks);

      mylog("inside for loop %d\n", j);

      // wait to receive, or for a timeout
      if (select(sock + 1, &socks, NULL, NULL, &ack_t)) {
        unsigned char buf[10000];
        int buf_len = sizeof(buf);
        int received;
        if ((received = recvfrom(sock, &buf, buf_len, 0, (struct sockaddr *) &in, (socklen_t *) &in_len)) < 0) {
          perror("recvfrom");
          exit(1);
        }

        header *myheader = get_header(buf);

        if ((myheader->magic == MAGIC) && (myheader->ack == 1)) { // Took out (myheader->sequence >= sequence)
          mylog("[recv ack] %d\n", myheader->sequence);
          if(myheader->sequence > sequence)
            sequence = myheader->sequence;
          verify(sequence);
          // Sliding window (Tahoe)
          if(window < slow_start)
            window = window*2;
          else
            window = window + 1;

          // receiver's buffer
          if(window > myheader->window)
            window = myheader->window;
         // done = 1;
        } else {
        
          //if(myheader->magic != MAGIC) mylog("magic is wrong %d=/=%d\n", myheader->magic, MAGIC);
          if(myheader->sequence < sequence) mylog("sequence is wrong %d < %d\n", myheader->sequence, sequence);
          if(myheader->ack != 1) mylog("this isn't an ack: %d\n", myheader->ack);

          mylog("[recv corrupted ack] %x %d\n", MAGIC, sequence);
        }
      } else {
        mylog("[error] dropped packet %d\n", j); 
        window = window/2;
        slow_start = window;
        break;
      }
    }
    }
    else{
      mylog("[error] timeout occurred\n");
    }
  }

  send_final_packet(sequence, sock, out);
  mylog("[completed]\n");

  return 0;
}
