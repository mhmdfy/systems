#include <math.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "disk.h"

#define RESCUERS

//int MAGIC; 

// Structure for Volume Control Block (VCB)
// It is the first block, and has the information for the file system.
typedef struct vcb_s {
  // a magic number to identify your disk
  int magic;
  int crashed;

  //description of the disk layout
  int blocksize;
  int de_start;
  int de_length;
  int fat_start;
  int fat_length;
  int db_start;
  
  // metadata for the root directory
  uid_t user;
  gid_t group;
  mode_t mode;
  struct timespec access_time;
  struct timespec modify_time;
  struct timespec create_time;
} vcb;

// Structure for Directory Entries (DE)
// They are the blocks that have data on each file in the system.
typedef struct de_s {
  unsigned int valid;
  unsigned int first_block;
  unsigned int size;

  // meadata for the file
  uid_t user;
  gid_t group;
  mode_t mode;
  struct timespec access_time;
  struct timespec modify_time;
  struct timespec create_time;
  char name[27];
} de;

// Structure for File Allocation Table (FAT)
// It is a table of pointers to the actual data.
typedef struct fatent_s {
  unsigned int used:1;
  unsigned int eof:1;
  unsigned int next:30;
} fatent;

int getMagic();

void writeVCB(vcb myvcb);

vcb readVCB();

vcb vcbSetUp(int size);

void deSetUp(int i);

void writeDE(int i, de myde);
