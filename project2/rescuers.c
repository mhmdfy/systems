
#include "rescuers.h"

//MAGIC = 1971083015;

int getMagic() { return  1971083015; }

// Write into the vcb
void writeVCB(vcb myvcb){
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  memcpy(tmp, &myvcb, sizeof(vcb));
  if(dwrite(0, tmp) < 0)
    perror("Error while writing to disk");
}

// Read from the vcb
vcb readVCB(){
  vcb myvcb;
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  if(dread(0, tmp) < 0)
    perror("Error while readin to disk");
  memcpy(&myvcb, tmp, sizeof(vcb));
  return myvcb;
}

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
  myvcb.de_length = 100;//(size * 15/100)-1;
  myvcb.fat_start = 101;//(size * 15/100);
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

void deSetUp(int i) {
  de myde;
  //myde.name = {};
  myde.valid = 0;
  myde.first_block = 0;
  myde.size = 0;

  myde.user = getuid();
  myde.group = getgid();
  myde.mode = 0777;
  clock_gettime(CLOCK_REALTIME, &myde.access_time);
  clock_gettime(CLOCK_REALTIME, &myde.modify_time);
  clock_gettime(CLOCK_REALTIME, &myde.modify_time);

  writeDE(i, myde);
}

void writeDE(int i, de myde) {
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  memcpy(tmp, &myde, sizeof(de));
  if(dwrite(i, tmp) < 0)
    perror("Error while writing to disk");
}

de readDE(int i){
   de myde;
  char tmp[BLOCKSIZE];
  memset(tmp, 0, BLOCKSIZE);
  if(dread(i, tmp) < 0)
    perror("Error while readin to disk");
  memcpy(&myde, tmp, sizeof(de));
  return myde;
}
