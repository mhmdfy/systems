/*
 * CS3600, Spring 2013
 * Project 4 Starter Code
 * (c) 2013 Alan Mislove
 *
 */

#ifndef __3600SENDRECV_H__
#define __3600SENDRECV_H__

#include <stdio.h>
#include <stdarg.h>

typedef struct header_t {
  unsigned int magic:30;
  unsigned int ack:1;
  unsigned int eof:1;
  unsigned short length;
  unsigned short window;
  unsigned int sequence;
} header;

typedef struct ooo_t {
  unsigned int sequence;
  unsigned int length;
} ooo;

unsigned int MAGIC;

void dump_packet(unsigned char *data, int size);
header *make_header(int sequence, int length, int eof, int ack, int window);
header *get_header(void *data);
char *get_data(void *data);
char *timestamp();
void mylog(char *fmt, ...);

#endif

