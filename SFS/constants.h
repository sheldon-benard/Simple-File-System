//
// Created by Sheldon Benard
//

#ifndef PROJECT_CONSTANTS_H
#define PROJECT_CONSTANTS_H

//---Include libraries for the project---//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include <stdint.h>

//---define static constants for the project---//

#define SFS_NAME "simple_file_system"

#define MAX_NUM_DIRECTORIES 1 //1 root directory
#define ROOT_DIR 0;

#define MAX_NUM_FILES 50 //We'll have max of 50 files

#define MAX_NUM_INODES MAX_NUM_DIRECTORIES+MAX_NUM_FILES //1 inode per file && directory

#define NEW_FILE -10 //We'll use this special flag to indicate a new file is being created
#define UNINITIALIZED_POINTER -1 //We'll use this special flag to indicate a uninitialized pointer
#define NO_FILE -1
#define FULL_IT -1
#define FULL_DT -1
#define FULL_FDT -1
#define NOT_IN_FDT -1

#define BLOCKSIZE 1024 //as per assignment recommendation
#define BLOCKSIZE_DOUBLE 1024.0 //for calculations

//We'll define a (MegaByte_SFS)MB File System;
#define MegaByte_SFS 5
#define NUM_BLOCKS BLOCKSIZE*MegaByte_SFS

//File name specification
#define MAX_FILE_NAME 16
#define MAX_FILE_EXT 3
#define PERIOD_IN_FILE_NAME 1
#define NULL_IN_FILE_NAME 1

#define FILE_TABLE_SIZE 32 //There will be 32 open files at any given time
#define ITABLE_NUM_BLOCKS 32 //Inode table will hold 32 blocks
#define DIRECTORY_NUM_BLOCKS 8 //Directory will hold 8 blocks
#define BITFIELD_BLOCKS 4 //Free bit map will hold 4 blocks

//Define direct pointers here; NUM_INDIR_PTRS will be a const int
#define NUM_DIR_PTRS 12

//define superblock constants
#define MAGIC_BLOCK 0xACBD0005
#define SUPER_BLOCK_INDEX 0

//define INDEX of important structures in data blocks
#define ITABLE_START_INDEX 1 //this is 32 blocks
#define DIRECTORY_BLOCK_START_INDEX 33 //this is 8 blocks
#define FREE_BIT_MAP_START_INDEX 41 //this is 4 blocks
#define DATA_BLOCK_START_INDEX 45

//Constants
static int const NUM_INDIR_PTRS = BLOCKSIZE/sizeof(int); //Indirect pointers: are int which are x bits, depending on comp

//With direct and indirect pointers defined, we can define max file length

static int const MAX_FILE_BLOCKS = NUM_DIR_PTRS + NUM_INDIR_PTRS;
static int const MAX_FILE_SIZE = BLOCKSIZE*MAX_FILE_BLOCKS;
static int const DATA_BLOCK_SIZE_IN_BYTES = ((NUM_BLOCKS - DATA_BLOCK_START_INDEX)/8) + 1;
static int const DATA_BLOCK_SIZE_IN_BITS = DATA_BLOCK_SIZE_IN_BYTES*8;

#endif //PROJECT_CONSTANTS_H
