Emily Montgomery
Mohammad Al Yahya

We have chosen the FAT disk layout beacuse it is easier to implement and debug. It is also
a well known file system and used currently by Mac OSX. 

It is laid out such that there is one block for volume control on the disk which contains
the metadata for the filesystem. Then there are a number of blocks that contain directory
entries. Each entry represents one file in the system. There are a number of block that
are used for the file allocation table. The FAT is a list of data blocks that say which
data blocks follow which in the layout of files. Then the rest of the blockes are
reserved for the actual user data.
