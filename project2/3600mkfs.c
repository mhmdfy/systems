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

  /* 3600: FILL IN CODE HERE.  YOU SHOULD INITIALIZE ANY ON-DISK
           STRUCTURES TO THEIR INITIAL VALUE, AS YOU ARE FORMATTING
           A BLANK DISK.  YOUR DISK SHOULD BE size BLOCKS IN SIZE. */

  /* 3600: AN EXAMPLE OF READING/WRITING TO THE DISK IS BELOW - YOU'LL
           WANT TO REPLACE THE CODE BELOW WITH SOMETHING MEANINGFUL. */

  // Writing VCB to the first block of the disk.
  writeVCB(vcbSetUp(size));

  // write a zero-ed block to each other block of the disk.
  char *tmp = (char *) malloc(BLOCKSIZE);
  memset(tmp, 0, BLOCKSIZE);

  for (int i=1; i<size; i++) 
    if (dwrite(i, tmp) < 0) 
      perror("Error while writing to disk");
  // voila! we now have a disk containing all zeros

  // Do not touch or move this function
  dunconnect();
}
