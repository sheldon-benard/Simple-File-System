//
// Created by Sheldon Benard on 2016-12-01.
//

#ifndef SFS_PROJECT_SIGNATURES_H
#define SFS_PROJECT_SIGNATURES_H

#include "constants.h"
#include "structs.h"

//bitfield signatures:

void set_bit_1(bitfield* bf, int position); //set bit at position to 1
int get_bit(bitfield* bf, int position); //get bit specified at position
void set_bit_0(bitfield* bf, int position); //set bit at position to free = 0
int find_free_bit(bitfield* bf); //find bit == 0 -> free bit

//inode signatures

void init_inode(inode* i); //Will set inode to initial conditions

//inode table signatures

void setUnoccupied_IT(inode_table* it, int index); //set occupied[index] = 0
void setOccupied_IT(inode_table* it, int index); //set occupied[index] = 1
int isIndexUnoccupied_IT(inode_table* it, int index); //does occupied[index] == 0
int getFreeEntry_IT(inode_table* it); //find unoccupied index
void deleteEntry_IT(inode_table* it, int index); //delete entry at index

//File Descriptor table signatures

int indexInode_FDT(FDT* fdt, int inode); //finds the index of an inode in the table
void setOccupied_FDT(FDT* fdt, int index); //sets the entry as active/occupied
void setUnoccupied_FDT(FDT* fdt, int index); //sets the entry as inactive/unoccupied
int isOccupied_FDT(FDT* fdt, int index); //checks to see if entry is active
int findFreeEntry_FDT(FDT* fdt); //return index of inactive entry

//Directory Table signatures

void setUnoccupied_DT(dir_table* dt, int index); //set occupied_entry[index] = 0
void setOccupied_DT(dir_table* dt, int index); //set occupied_entry[index] = 1
int isIndexUnoccupied_DT(dir_table* dt, int index); //is occupied_entry[index] == 0
int getFreeEntry_DT(dir_table* dt); //return index of unoccupied entry
int getIndexByName_DT(dir_table* dt, char* name); //use name to find index if DT
void deleteEntry_DT(dir_table* dt, int index); //delete entry in DT
int validFileName(char* name); //Check if filename is valid

#endif //SFS_PROJECT_SIGNATURES_H
