/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This program is intended to format your disk file, and should be executed
 * BEFORE any attempt is made to mount your file system.  It will not, however
 * be called before every mount (you will call it manually when you format 
 * your disk file).
 */

#include "3600mkfs.h"


int main(int argc, char** argv) {
  // Do not touch this function
  if (argc != 2) {
    printf("Invalid number of arguments \n");
    printf("usage: %s diskSizeInBlockSize\n", argv[0]);
    return 1;
  }

  unsigned long size = atoi(argv[1]);
  printf("Formatting the disk with size %lu \n", size);
  myformat(size);
}

void myformat(int size) {
  // Do not touch or move this function
  dcreate_connect();

  // Writing VCB to the first block of the disk.
  vcb myvcb = vcbSetUp(size);

  for (int i=myvcb.de_start; i<myvcb.de_length; i++) 
    deSetUp(i);

  // Do not touch or move this function
  dunconnect();
}
