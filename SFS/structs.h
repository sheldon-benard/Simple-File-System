//
// Created by Sheldon Benard on 2016-12-01.
//

#ifndef SFS_PROJECT_STRUCTS_H
#define SFS_PROJECT_STRUCTS_H

#include "constants.h"

//Define bitfield struct -> to be used to find free data blocks amongst all data blocks available to programs

typedef struct {
    unsigned char field[DATA_BLOCK_SIZE_IN_BYTES]; //will be the actual bitfield -> char is 1 byte, so we'll have 8*DATA_BLOCK_SIZE_IN_BYTES bits
    int last_set_bit; //keeping track of a previous bit set to 1 will help with searching for free data blocks
}bitfield;

//Define superblock struct -> to be initialized and put in first data block for reference

typedef struct {
    int magic_block;
    int block_size;
    int file_system_size;
    int inode_table_length;
    int root_dir;
}superblock;

//Inode to be used as entry in inode table; the fields are those defined in the assignment description

typedef struct {
    int mode;
    int link_cnt;
    int uid;
    int gid;
    int size;
    int pointer[NUM_DIR_PTRS]; //NUM_DIR_PTRS of direct pointers
    int ind_pointer; //ind_pointer will point at data_block of pointers
}inode;

//Inode table; i[] will be array of inodes, and occupied_inode[x] == 1 if inode is taken//not usable

typedef struct{
    inode i[MAX_NUM_INODES];
    int occupied_inode[MAX_NUM_INODES];
}inode_table;

//Indirect block; this struct will be the model for a data block whose
//bytes are used to point to other data blocks

typedef struct{
    int block[NUM_INDIR_PTRS];
}indir_ptrs;

//File Directory Table entry:

typedef struct{
    int inode; //inode number associated with file
    int rd_ptr; //read pointer
    int wr_ptr; //write pointer
} FDT_entry;

//File directory table:

typedef struct{
    FDT_entry entries[FILE_TABLE_SIZE]; //array of FDT_entries so we can access files by FILEID
    int occupied[FILE_TABLE_SIZE]; //array of active FDT slots, so we can access free slots
}FDT;

//Directory entry -> specifies the file name and inode_num associated with file name

typedef struct{
    char file_name[MAX_FILE_NAME + PERIOD_IN_FILE_NAME + MAX_FILE_EXT + NULL_IN_FILE_NAME];
    int inode_num;
}dir_entry;

//Directory table:

typedef struct{
    dir_entry directory[MAX_NUM_INODES]; //directory array of dir_entries
    char occupied_entry[MAX_NUM_INODES]; //occupied_entry will keep track of free slots in dir_table
    int directory_ptr; //keeps track for the get next file call
}dir_table;














#endif //SFS_PROJECT_STRUCTS_H
