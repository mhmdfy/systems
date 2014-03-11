
#include "rescuers.h"

int getMagic() { return  1971083015; }

// Sets up the vcb
vcb vcbSetUp(int size) {
  // size is going to be used later to figure out the lengths
  // this line is just to make the warning disapear.
  size = size;
  vcb myvcb;
  myvcb.magic = getMagic();
  myvcb.crashed = 0;
  myvcb.blocksize = BLOCKSIZE;
  myvcb.de_start = 1;
  myvcb.de_length = (size * 10/100); // 100
  myvcb.fat_start = (size * 10/100) + 1; //101;
  myvcb.fat_length = 7;//(size * 20/100)-1;
  myvcb.db_start = 108;//(size * 20/100);

  myvcb.user = getuid();
  myvcb.group = getgid();
  myvcb.mode = 0777;
  clock_gettime(CLOCK_REALTIME, &myvcb.access_time);
  clock_gettime(CLOCK_REALTIME, &myvcb.modify_time);
  clock_gettime(CLOCK_REALTIME, &myvcb.modify_time);

 return myvcb;
}

// Create the setup for de (block i)
void deSetUp(int i) {
  de myde;
  myde.valid = 0;
  myde.first_block = -1;
  myde.size = 0;

  myde.user = getuid();
  myde.group = getgid();
  myde.mode = 0777;
  clock_gettime(CLOCK_REALTIME, &myde.access_time);
  clock_gettime(CLOCK_REALTIME, &myde.modify_time);
  clock_gettime(CLOCK_REALTIME, &myde.modify_time);

  writeDE(i, myde);
}

// Read from the vcb
vcb readVCB(){
  vcb myvcb;
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  readDATA(0, tmp);
  memcpy(&myvcb, tmp, sizeof(vcb));
  return myvcb;
}

// Write into the vcb
void writeVCB(vcb myvcb){
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  memcpy(tmp, &myvcb, sizeof(vcb));
  writeDATA(0, tmp);
}

// Read from de (block i)
de readDE(int i){
  de myde;
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  readDATA(i, tmp);
  memcpy(&myde, tmp, sizeof(de));
  return myde;
}

// Write into de (block i)
void writeDE(int i, de myde) {
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  memcpy(tmp, &myde, sizeof(de));
  writeDATA(i, tmp);
}

// Read from fat entry i
fat readFAT(int i, int fatstart){
  int blocknum = i/128 + fatstart;
  int blockent = i%128;

  fat *tmp = (fat*) malloc(BLOCKSIZE);

  memset(tmp, 0, BLOCKSIZE);
  readDATA(blocknum, (void*) tmp);

  return tmp[blockent];
}

// Write into fat entry i
void writeFAT(int i, fat myfat, int fatstart) {
  int blocknum = i/128 + fatstart;
  int blockent = i%128;

  fat *tmp = (fat*) malloc(BLOCKSIZE);

  memset(tmp, 0, BLOCKSIZE);
  readDATA(blocknum, (void*) tmp);

  tmp[blockent] = myfat;

  writeDATA(blocknum, (void*) tmp);
}

// Read data from block i
void readDATA(int i, char* data){
  if(dread(i, data) < 0)
    perror("Error while readin to disk");
}

// Write data into block i
void writeDATA(int i, char* data) {
  if(dwrite(i, data) < 0)
    perror("Error while writing to disk");
}

