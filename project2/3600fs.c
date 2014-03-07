/*
 * CS3600, Spring 2014
 * Project 2 Starter Code
 * (c) 2013 Alan Mislove
 *
 * This file contains all of the basic functions that you will need 
 * to implement for this project.  Please see the project handout
 * for more details on any particular function, and ask on Piazza if
 * you get stuck.
 */

#define FUSE_USE_VERSION 26

#ifdef linux
/* For pread()/pwrite() */
#define _XOPEN_SOURCE 500
#endif

#define _POSIX_C_SOURCE 199309
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include "3600fs.h"

vcb MYVCB;

/*
 * Initialize filesystem. Read in file system metadata and initialize
 * memory structures. If there are inconsistencies, now would also be
 * a good time to deal with that. 
 *
 * HINT: You don't need to deal with the 'conn' parameter AND you may
 * just return NULL.
 *
 */
static void* vfs_mount(struct fuse_conn_info *conn) {
  fprintf(stderr, "vfs_mount called\n");

  // Do not touch or move this code; connects the disk
  dconnect();

  /* 3600: YOU SHOULD ADD CODE HERE TO CHECK THE CONSISTENCY OF YOUR DISK
           AND LOAD ANY DATA STRUCTURES INTO MEMORY */
  MYVCB = readVCB();
  if(MYVCB.magic != getMagic()){
    perror("The file system is not me\n");
    exit(-1);
  }

  if(MYVCB.crashed == 0)
    MYVCB.crashed = 1;
  else {
    perror("The file system crashed last time and could be currepted\n");
    exit(-1);
  }
  writeVCB(MYVCB);

  return NULL;
}

/*
 * Called when your file system is unmounted.
 *
 */
static void vfs_unmount (void *private_data) {
  fprintf(stderr, "vfs_unmount called\n");

  /* 3600: YOU SHOULD ADD CODE HERE TO MAKE SURE YOUR ON-DISK STRUCTURES
           ARE IN-SYNC BEFORE THE DISK IS UNMOUNTED (ONLY NECESSARY IF YOU
           KEEP DATA CACHED THAT'S NOT ON DISK */
  MYVCB.crashed = 0;
  writeVCB(MYVCB);

  // Do not touch or move this code; unconnects the disk
  dunconnect();
}

/* 
 *
 * Given an absolute path to a file/directory (i.e., /foo ---all
 * paths will start with the root directory of the CS3600 file
 * system, "/"), you need to return the file attributes that is
 * similar stat system call.
 *
 * HINT: You must implement stbuf->stmode, stbuf->st_size, and
 * stbuf->st_blocks correctly.
 *
 */
static int vfs_getattr(const char *path, struct stat *stbuf) {
  fprintf(stderr, "vfs_getattr called\n");

  // Do not mess with this code 
  stbuf->st_nlink = 1; // hard links
  stbuf->st_rdev  = 0;
  stbuf->st_blksize = BLOCKSIZE;

  /* 3600: YOU MUST UNCOMMENT BELOW AND IMPLEMENT THIS CORRECTLY */
  de myde;
  int i;
  int found = 0;
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++){
    myde = readDE(i);
    if(strcmp(myde.name, path) == 0){
      found = 1;
      break;
    }
  }

  if(!found || (myde.valid == 0)){
    perror("This file does not exist.\n");
    return -ENOENT;
  }

  if (strcmp(path, "/")  == 0)//The path represents the root directory)
    stbuf->st_mode  = 0777 | S_IFDIR;
  else 
    stbuf->st_mode  = (myde.mode & 0x0000ffff) | S_IFREG;

  stbuf->st_uid     = myde.user;// file uid
  stbuf->st_gid     = myde.group;// file gid
  stbuf->st_atime   = myde.access_time.tv_sec;// access time 
  stbuf->st_mtime   = myde.modify_time.tv_sec;// modify time
  stbuf->st_ctime   = myde.create_time.tv_sec;// create time
  stbuf->st_size    = myde.size;// file size
  stbuf->st_blocks  = myde.size/BLOCKSIZE;// file size in blocks
    

  return 0;
}

/*
 * Given an absolute path to a directory (which may or may not end in
 * '/'), vfs_mkdir will create a new directory named dirname in that
 * directory, and will create it with the specified initial mode.
 *
 * HINT: Don't forget to create . and .. while creating a
 * directory.
 */
/*
 * NOTE: YOU CAN IGNORE THIS METHOD, UNLESS YOU ARE COMPLETING THE 
 *       EXTRA CREDIT PORTION OF THE PROJECT.  IF SO, YOU SHOULD
 *       UN-COMMENT THIS METHOD.
static int vfs_mkdir(const char *path, mode_t mode) {

  return -1;
} */

/** Read directory
 *
 * Given an absolute path to a directory, vfs_readdir will return 
 * all the files and directories in that directory.
 *
 * HINT:
 * Use the filler parameter to fill in, look at fusexmp.c to see an example
 * Prototype below
 *
 * Function to add an entry in a readdir() operation
 *
 * @param buf the buffer passed to the readdir() operation
 * @param name the file name of the directory entry
 * @param stat file attributes, can be NULL
 * @param off offset of the next entry or zero
 * @return 1 if buffer is full, zero otherwise
 * typedef int (*fuse_fill_dir_t) (void *buf, const char *name,
 *                                 const struct stat *stbuf, off_t off);
 *			   
 * Your solution should not need to touch fi
 *
 */
static int vfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi) {
  printf("path is: %s\n", path);
  if(strcmp(path, "/") == 0){
    de myde;
    int i;
    for(i = MYVCB.de_start + offset; i < MYVCB.de_start + MYVCB.de_length; i++){
      myde = readDE(i);
      if(myde.valid) {
        printf("file is valid: %s\n", myde.name);
        char* string = myde.name + 1;
        if(filler(buf, string, NULL, i + 1 - MYVCB.de_start) != 0)
          return 0;
      }
    }
    return 0;
  }
  perror("Directory does not exist.\n");
  return -1;
}

/*
 * Given an absolute path to a file (for example /a/b/myFile), vfs_create 
 * will create a new file named myFile in the /a/b directory.
 *
 */
static int vfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
  de myde;
  int i;
  int found = 0;
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++){
    myde = readDE(i);
    if(strcmp(myde.name, path) == 0){
      found = 1;
      break;
    }
  }

  if(found){
    perror("This file already exists.\n");
    return -1;
  }
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++){
    myde = readDE(i);
    if(myde.valid == 0){
      myde.valid = 1;
      myde.user = geteuid();
      myde.group = getegid();
      clock_gettime(CLOCK_REALTIME, &myde.access_time);
      clock_gettime(CLOCK_REALTIME, &myde.modify_time);
      clock_gettime(CLOCK_REALTIME, &myde.create_time);
      myde.mode = mode;
      strcpy(myde.name, path);
      writeDE(i, myde);
      return 0;
    }
  }
  perror("The disk is full.\n");
  return -1;
}

/*
 * The function vfs_read provides the ability to read data from 
 * an absolute path 'path,' which should specify an existing file.
 * It will attempt to read 'size' bytes starting at the specified
 * offset (offset) from the specified file (path)
 * on your filesystem into the memory address 'buf'. The return 
 * value is the amount of bytes actually read; if the file is 
 * smaller than size, vfs_read will simply return the most amount
 * of bytes it could read. 
 *
 * HINT: You should be able to ignore 'fi'
 *
 */
static int vfs_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi){
  de myde;
  int i;
  int read = 0;
  fat myfat;
  int index;
  char* data = (char*) calloc(BLOCKSIZE, sizeof(char));
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++){
    myde = readDE(i);
    if(strcmp(myde.name, path) == 0 && myde.valid){
      if(myde.first_block != 0){
        index = myde.first_block;
        while(offset >= 512){
          myfat = readFAT(index, MYVCB.fat_start);
          if(myfat.used && !myfat.eof){
            index = myfat.next;
          }else{
            perror("Data does not exist.\n");
            return -1;
          }
          offset = offset-512;
        }
        while(size > 0){
          if(*data == '\0'){
            myfat = readFAT(index, MYVCB.fat_start);
            index = myfat.next;
          }
          readDATA(index, data);
          data = data + offset;
          offset = 0;
          while(*data != '\0' && size > 0){
            *buf = * data;
            data++;
            buf++;
            read++;
            size--;
          }
          *buf = '\0';
        }
      }
     clock_gettime(CLOCK_REALTIME, &myde.access_time);
     writeDE(i, myde);
     return read;
    }
  }
  perror("The file does not exist.\n");
  return -1;
}

static int newBlock(int index, fat prev) {
  int i;
  fat myfat;
  for(i = 0; i < MYVCB.fat_length*128; i++){
    myfat = readFAT(i, MYVCB.fat_start);
    if(!myfat.used) {
      myfat.used = 1;
      myfat.eof = 1;
      char* data = (char*) calloc(BLOCKSIZE, sizeof(char));
      writeDATA(i, data);
      // free(data);
      writeFAT(i, myfat, MYVCB.fat_start);
      break;
    }
  }
  prev.next = i;
  writeFAT(index, prev, MYVCB.fat_start);
  return i;
}

/*
 * The function vfs_write will attempt to write 'size' bytes from 
 * memory address 'buf' into a file specified by an absolute 'path'.
 * It should do so starting at the specified offset 'offset'.  If
 * offset is beyond the current size of the file, you should pad the
 * file with 0s until you reach the appropriate length.
 *
 * You should return the number of bytes written.
 *
 * HINT: Ignore 'fi'
 */
static int vfs_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{

  /* 3600: NOTE THAT IF THE OFFSET+SIZE GOES OFF THE END OF THE FILE, YOU
           MAY HAVE TO EXTEND THE FILE (ALLOCATE MORE BLOCKS TO IT). */
  //TODO size!
  de myde;
  int i;
  int writen = 0;
  fat myfat;
  int index;
  char* data = (char*) calloc(BLOCKSIZE, sizeof(char));
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++ ) {
    myde = readDE(i);
    if(myde.valid && strcmp(myde.name, path) == 0) {
      index = myde.first_block;
      while(offset >= 512) {
        myfat = readFAT(index, MYVCB.fat_start);
        if(myfat.used && !myfat.eof) {
          index = myfat.next;
        }
        else {
          index = newBlock(index, myfat);
        }
        offset = offset - 512;
      }
      while(size > 0) {
        if(*data == '\0') {
          myfat = readFAT(index, MYVCB.fat_start);
          index = newBlock(index, myfat);
        }
        data = data + offset;
        offset = 0;
        while(*data != '\0' && *buf != '\0' && size > 0) {
          *data = *buf;
          data++;
          buf++;
          writen++;
          size--;
        }
        *data = '\0';
        writeDATA(index, data);
      }
      clock_gettime(CLOCK_REALTIME, &myde.access_time);
      clock_gettime(CLOCK_REALTIME, &myde.modify_time);
      writeDE(i, myde);
      return writen;
    }
  }
  perror("The file does not exist.\n");
  return -1;
}

static void cleanBlocks(int index)
{
  if(index != 0) {
    fat myfat = readFAT(index, MYVCB.fat_start);
    while(myfat.used) {
      if(!myfat.eof) 
        index = myfat.next;
      myfat.used = 0;
      writeFAT(index, myfat, MYVCB.fat_start);
      myfat = readFAT(index, MYVCB.fat_start);
    }
  }
}

/**
 * This function deletes the last component of the path (e.g., /a/b/c you 
 * need to remove the file 'c' from the directory /a/b).
 */
static int vfs_delete(const char *path)
{

  /* 3600: NOTE THAT THE BLOCKS CORRESPONDING TO THE FILE SHOULD BE MARKED
           AS FREE, AND YOU SHOULD MAKE THEM AVAILABLE TO BE USED WITH OTHER FILES */
  de myde;
  int i;
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++){
    myde = readDE(i);
    if(strcmp(myde.name, path) == 0){
      myde.valid = 0;
      cleanBlocks(myde.first_block);
      writeDE(i, myde);
      return 0;
    }
  }
  perror("File not found.\n");
  return -1;
}

/*
 * The function rename will rename a file or directory named by the
 * string 'oldpath' and rename it to the file name specified by 'newpath'.
 *
 * HINT: Renaming could also be moving in disguise
 *
 */
static int vfs_rename(const char *from, const char *to)
{
  de myde;
  int i;
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++){
    myde = readDE(i);
    if(strcmp(myde.name, from) == 0){
      strcpy(myde.name, to);
      writeDE(i, myde);
      return 0;
    }
  }
  perror("File not found.\n");
  return -1;
}


/*
 * This function will change the permissions on the file
 * to be mode.  This should only update the file's mode.  
 * Only the permission bits of mode should be examined 
 * (basically, the last 16 bits).  You should do something like
 * 
 * fcb->mode = (mode & 0x0000ffff);
 *
 */
static int vfs_chmod(const char *file, mode_t mode)
{
  de myde;
  int i;
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++){
    myde = readDE(i);
    if(strcmp(myde.name, file) == 0){
      myde.mode = (mode & 0x0000ffff);    
      writeDE(i, myde);
      return 0;
    }
  }
  perror("File not found.\n");
  return -1;
}

/*
 * This function will change the user and group of the file
 * to be uid and gid.  This should only update the file's owner
 * and group.
 */
static int vfs_chown(const char *file, uid_t uid, gid_t gid)
{
  de myde;
  int i;
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++){
    myde = readDE(i);
    if(strcmp(myde.name, file) == 0){
      myde.user = uid;
      myde.group = gid;
      writeDE(i, myde);
      return 0;
    }
  }
  perror("File not found.\n");
  return -1;
}

/*
 * This function will update the file's last accessed time to
 * be ts[0] and will update the file's last modified time to be ts[1].
 */
static int vfs_utimens(const char *file, const struct timespec ts[2])
{
  de myde;
  int i;
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++){
    myde = readDE(i);
    if(strcmp(myde.name, file) == 0){
      myde.access_time = ts[0];
      myde.modify_time = ts[1];
      writeDE(i, myde);
      return 0;
    }
  }
  perror("File not found.\n");
  return -1;
}

/*
 * This function will truncate the file at the given offset
 * (essentially, it should shorten the file to only be offset
 * bytes long).
 */
static int vfs_truncate(const char *file, off_t offset)
{

  /* 3600: NOTE THAT ANY BLOCKS FREED BY THIS OPERATION SHOULD
           BE AVAILABLE FOR OTHER FILES TO USE. */
  de myde;
  int i;
  fat myfat;
  int index;
  char* data = (char*) calloc(BLOCKSIZE, sizeof(char));
  for(i = MYVCB.de_start; i < MYVCB.de_start + MYVCB.de_length; i++) {
    myde = readDE(i);
    if(strcmp(myde.name, file) == 0) {
      myde.size = offset;
      index = myde.first_block;
      while(offset >= 512) {
        myfat = readFAT(index, MYVCB.fat_start);
        if(myfat.used && !myfat.eof) {
          index = myfat.next;
        }
        else {
          perror("Offset is larger than file.\n");
        }
        offset = offset - 512;
      }
      readDATA(index, data);
      *(data+offset) = '\0';
      writeDATA(index, data);
      myfat = readFAT(index, MYVCB.fat_start);  
      cleanBlocks(myfat.next);
      writeDE(i, myde);
      return 0;
    }
  }
  perror("File not found.\n");
  return -1;
}


/*
 * You shouldn't mess with this; it sets up FUSE
 *
 * NOTE: If you're supporting multiple directories for extra credit,
 * you should add 
 *
 *     .mkdir	 = vfs_mkdir,
 */
static struct fuse_operations vfs_oper = {
    .init    = vfs_mount,
    .destroy = vfs_unmount,
    .getattr = vfs_getattr,
    .readdir = vfs_readdir,
    .create	 = vfs_create,
    .read	 = vfs_read,
    .write	 = vfs_write,
    .unlink	 = vfs_delete,
    .rename	 = vfs_rename,
    .chmod	 = vfs_chmod,
    .chown	 = vfs_chown,
    .utimens	 = vfs_utimens,
    .truncate	 = vfs_truncate,
};

int main(int argc, char *argv[]) {
    /* Do not modify this function */
    umask(0);
    if ((argc < 4) || (strcmp("-s", argv[1])) || (strcmp("-d", argv[2]))) {
      printf("Usage: ./3600fs -s -d <dir>\n");
      exit(-1);
    }
    return fuse_main(argc, argv, &vfs_oper, NULL);
}

