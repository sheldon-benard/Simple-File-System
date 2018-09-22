//
// Created by Sheldon Benard on 2016-12-01.
//

#ifndef SFS_PROJECT_STRUCT_FUNCTIONS_H
#define SFS_PROJECT_STRUCT_FUNCTIONS_H

//import structs, constants, and signatures
#include "structs.h"
#include "signatures.h"
#include "constants.h"

//--------------- BITFIELD FUNCTIONS -------------------//

//set_bit at position to 1
void set_bit_1(bitfield* bf, int position){
    int byte = position/8;
    int bit = position%8;
    bf->field[byte] |= (1 << (bit));
}
//get_bit at position
int get_bit(bitfield* bf, int position){
    int byte = position/8;
    int bit = position%8;
    //if bf->field changes after being & with ~(1 << bit), then we have a one in the bit location
    if(bf->field[byte] != (bf->field[byte] & ~(1 << (bit)))) return 1;
    else return 0; //else, we have a 0 in the bit location
}
//set_bit at position to 0
void set_bit_0(bitfield* bf, int position){
    int byte = position/8;
    int bit = position%8;
    bf->field[byte] &= ~(1 << (bit));
}
//find a free bit; use struct last_set_bit to help
int find_free_bit(bitfield* bf){
    int i, ret = -1, bit;
    for(i = 0; i<DATA_BLOCK_SIZE_IN_BITS;i++){
        bit = get_bit(bf, (bf->last_set_bit + i) % DATA_BLOCK_SIZE_IN_BITS);
        if(bit == 0){
            bf->last_set_bit = (bf->last_set_bit + i) % DATA_BLOCK_SIZE_IN_BITS;
            ret = bf->last_set_bit;
            break;
        }
    }
    return ret; //if ret != -1, then we found a free bit
}

//--------------- INODE FUNCTIONS -------------------//

void init_inode(inode* i){
    //Set all ints to 0
    i->mode = 0;
    i->link_cnt = 0;
    i->uid = 0;
    i->gid = 0;
    i->size = 0;

    //Set all pointers to UNINITIALIZED_POINTER
    int x;
    for(x=0;x<NUM_DIR_PTRS;x++) i->pointer[x] = UNINITIALIZED_POINTER;

    i->ind_pointer = UNINITIALIZED_POINTER;
}

//--------------- INODE TABLE FUNCTIONS -------------------//
//set entry[index] as free
void setUnoccupied_IT(inode_table* it, int index){
    it->occupied_inode[index] = 0;
}
//set entry[index] as taken
void setOccupied_IT(inode_table* it, int index){
    it->occupied_inode[index] = 1;
}
//check is entry[index] free
int isIndexUnoccupied_IT(inode_table* it, int index){
    return it->occupied_inode[index] == 0;
}

//iterate through IT, find free index; return -1 if no free index
int getFreeEntry_IT(inode_table* it){
    int x;
    for(x = 0;x<MAX_NUM_INODES;x++){
        if(isIndexUnoccupied_IT(it, x)) return x;
    }
    return -1;
}

//delete entry: free the entry[index] and reset the inode
void deleteEntry_IT(inode_table* it, int index){
    setUnoccupied_IT(it, index);
    init_inode(&it->i[index]);
}

//--------------- FILE DESCRIPTOR TABLE FUNCTIONS -------------------//

//Each file descriptor table entry has inode, rd_ptr, wr_ptr
//So, only can use inode to find index == FILEID
int indexInode_FDT(FDT* fdt, int inode){
    //iterate through x; if entry[x].inode == inode, return x == FILEID
    int x;
    for(x = 0;x<FILE_TABLE_SIZE;x++)
        if (fdt->entries[x].inode == inode) return x;
    return -1; //no index with inode
};
//set entry[index] as occupied
void setOccupied_FDT(FDT* fdt, int index){
    fdt->occupied[index] = 1;
}
//set entry[index] as unoccupied
void setUnoccupied_FDT(FDT* fdt, int index){
    fdt->occupied[index] = 0;
}
//check if entry[index] is Occupied
int isOccupied_FDT(FDT* fdt, int index){
    return fdt->occupied[index] == 1;
}

int findFreeEntry_FDT(FDT* fdt){
    int x;
    for(x=0;x<FILE_TABLE_SIZE;x++){
        if(!isOccupied_FDT(fdt, x)) return x;
    }
    return -1; //no free spots
}

//--------------- DIRECTORY TABLE FUNCTIONS -------------------//
//check for valid name
int validFileName(char* name){
    if(strlen(name) == 0) return -1; // name length == 0, invalid
    int x = 0,y = 0;
    while(name[x] != '.'){
        if(x >= MAX_FILE_NAME || name[x] == '\0') return -1; //invalid file name
        x++;
    }
    x++; //if we are here, name[x] == '.', so increment up again
    while(name[x+y] != '\0'){
        if(y >= MAX_FILE_EXT) return -1; //invalid extension
        y++;
    }
    return 1; //made it to end, so valid file name
}

//set entry[index] to unoccupied
void setUnoccupied_DT(dir_table* dt, int index){
    dt->occupied_entry[index] = 0;
}

//set entry[index] tp occupied
void setOccupied_DT(dir_table* dt, int index){
    dt->occupied_entry[index] = 1;
}
int isIndexUnoccupied_DT(dir_table* dt, int index){
    return dt->occupied_entry[index] == 0;
}
int getFreeEntry_DT(dir_table* dt){
    int x;
    for(x = 0;x<MAX_NUM_INODES;x++){
        if(isIndexUnoccupied_DT(dt, x)) return x;
    }
    return -1;
}
int getIndexByName_DT(dir_table* dt, char* name){
    int x;
    for(x=0;x<MAX_NUM_INODES;x++){
        if(strcmp(name, dt->directory[x].file_name) == 0) return x;
    }
    return -1;
}
void deleteEntry_DT(dir_table* dt, int index){
    setUnoccupied_DT(dt, index);
    dt->directory[index].inode_num = 0;
    memset(dt->directory[index].file_name,'\0',MAX_FILE_NAME + 1 + MAX_FILE_EXT + 1);
}

#endif //SFS_PROJECT_STRUCT_FUNCTIONS_H
