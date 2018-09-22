//
// Created by Sheldon Benard on 2016-12-01.
//

#ifndef SFS_PROJECT_MY_SFS_API_H
#define SFS_PROJECT_MY_SFS_API_H

#include "disk_emu.h"
#include "constants.h"
#include "structs.h"
#include "signatures.h"
#include "struct_functions.h"

//Functions you should implement.
//Return -1 for error besides mksfs

void mksfs(int fresh);
int sfs_get_next_file_name(char *fname);
int sfs_get_file_size(char* path);
int sfs_fopen(char *name);
int sfs_fclose(int fileID);
int sfs_frseek(int fileID, int loc);
int sfs_fwseek(int fileID, int loc);
int sfs_fwrite(int fileID, char *buf, int length);
int sfs_fread(int fileID, char *buf, int length);
int sfs_remove(char *file);

void load(void* buffer, unsigned long int size, int index); //helper: loads data blocks when size in bytes
void save(void* buffer, unsigned long int size, int index); //helper: saves data blocks when size in bytes
void erase_block(int block_index); //will erase a block; helper for remove


#endif //SFS_PROJECT_MY_SFS_API_H
