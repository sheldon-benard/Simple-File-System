//
// Created by Sheldon Benard on 2016-11-29.
//

#include "my_sfs_api.h"

FDT* file_descriptor_table = 0;
inode_table* i_table = 0;
bitfield* free_block_map = 0;
dir_table* directory_table = 0;

//Create file system

void mksfs(int fresh){

    //As per assignment description, we'll put FDT, Data_block, IT, DT in memory
    //Check to see if memory already allocated before allocating more
    if(file_descriptor_table == 0) file_descriptor_table = malloc(sizeof(FDT));
    if(free_block_map == 0) free_block_map = malloc(sizeof(bitfield));
    if(i_table == 0) i_table = malloc(sizeof(inode_table));
    if(directory_table == 0) directory_table = malloc(sizeof(dir_table));
    //Clear data
    memset(file_descriptor_table, '\0', sizeof(FDT));
    memset(free_block_map, '\0', sizeof(bitfield));
    memset(i_table, '\0', sizeof(inode_table));
    memset(directory_table, '\0', sizeof(dir_table));

    //Clear tables entries; set FDT to all unoccupied

    int x;
    for(x = 0; x < MAX_NUM_INODES;x++)deleteEntry_DT(directory_table, x);
    for(x = 0; x < MAX_NUM_INODES;x++)deleteEntry_IT(i_table, x);
    for(x = 0; x < FILE_TABLE_SIZE; x++)setUnoccupied_FDT(file_descriptor_table, x);

    //if fresh is true, then we are creating new file system
    if(fresh){
        //initialize a new file system
        init_fresh_disk(SFS_NAME, BLOCKSIZE, NUM_BLOCKS);

        //Start with superblock
        superblock s = {}; //reset struct
        s.magic_block = MAGIC_BLOCK;
        s.block_size = BLOCKSIZE;
        s.file_system_size = NUM_BLOCKS;
        s.inode_table_length = ITABLE_NUM_BLOCKS;

        //get free inode for root_dir
        s.root_dir = getFreeEntry_IT(i_table); //entry == ROOT_DIR
        setOccupied_IT(i_table, s.root_dir);

        //Write super block index into data
        write_blocks(SUPER_BLOCK_INDEX, 1, &s);

        //Use helper functions -> will deal with changing sizeof in bytes to number of blocks
        save(free_block_map, sizeof(bitfield), FREE_BIT_MAP_START_INDEX);
        save(i_table, sizeof(inode_table), ITABLE_START_INDEX);
        save(directory_table, sizeof(dir_table), DIRECTORY_BLOCK_START_INDEX);
    }
    else{
        //fresh is false, we have a SFS already
        init_disk(SFS_NAME, BLOCKSIZE, NUM_BLOCKS);

        //Use helper functions -> will deal with changing sizeof in bytes to number of blocks
        load(free_block_map, sizeof(bitfield), FREE_BIT_MAP_START_INDEX);
        load(i_table, sizeof(inode_table), ITABLE_START_INDEX);
        load(directory_table, sizeof(dir_table), DIRECTORY_BLOCK_START_INDEX);

    }
}

void load(void* buffer, unsigned long int size, int index){
    //number_of_blocks taken by size will be ceil of size/BLOCKSIZE
    int number_of_blocks = (int)ceil(size/BLOCKSIZE_DOUBLE);
    unsigned long int actual_size = (unsigned long int)number_of_blocks*BLOCKSIZE;
    char* temp = calloc(actual_size,1); //actual_size >= size, so use this to ensure no error in memcpy

    //read_blocks from memory, then copy from temp to buffer
    read_blocks(index, number_of_blocks, temp);
    memcpy(buffer, temp, size);
    //free temp
    free(temp);
}

void save(void* buffer, unsigned long int size, int index){
    //number_of_blocks taken by size will be ceil of size/BLOCKSIZE
    int number_of_blocks = (int)ceil(size/BLOCKSIZE_DOUBLE);
    unsigned long int actual_size = (unsigned long int)number_of_blocks*BLOCKSIZE;
    char* toDisk = calloc(actual_size,1); //actual_size >= size, so use this to ensure no error in memcpy
    //copy data from buffer to toDisk
    memcpy(toDisk, buffer, size);
    //write data to disk
    write_blocks(index, number_of_blocks, toDisk);
    //free toDisk
    free(toDisk);
}

int sfs_get_next_file_name(char *fname){
    //We'll use the directory_ptr to increment through directory table
    while(directory_table->directory_ptr < MAX_NUM_INODES){
        if(!isIndexUnoccupied_DT(directory_table, directory_table->directory_ptr)){
            //thus, the pointer location has a file in it; copy name into fname, increment directory_ptr, return 1
            strcpy(fname, directory_table->directory[directory_table->directory_ptr].file_name);
            directory_table->directory_ptr++;
            return 1;
        }
        //else, nothing at table location[directory_ptr]; increment
        directory_table->directory_ptr++;
    }
    //thus, directory_ptr >= MAX_NUM_INODES -> reset to 0 and return 0 -> end of dir_table
    directory_table->directory_ptr = 0;
    return 0;
}

int sfs_get_file_size(char* path){
    //compare path/file name to each name in directory;
    //if match, then return size of the file
    int x;
    for(x=0;x<MAX_NUM_INODES;x++){
        if(strcmp(path,directory_table->directory[x].file_name) == 0){
            return i_table->i[directory_table->directory[x].inode_num].size;
        }
    }
    //No file with name == path
    return -1;
}

int sfs_fopen(char *name){
    if(validFileName(name) == -1) return -1; //invalid file name

    //Now, we have to get the index of the file in the directory;
    //if it's not there, we'll have to create the file
    int dt_index = getIndexByName_DT(directory_table, name);
    if(dt_index == NO_FILE){
        //file doesn't exist -> create the file

        //check if there is space in tables
        if(getFreeEntry_DT(directory_table) == FULL_DT) return -1; //FULL directory table
        if(getFreeEntry_IT(i_table) == FULL_IT) return -1; //FULL Inode Table
        if(findFreeEntry_FDT(file_descriptor_table) == FULL_FDT) return -1; //FULL FDT

        //Now, grab dt_index and set it occupied
        dt_index = getFreeEntry_DT(directory_table);
        setOccupied_DT(directory_table, dt_index);

        //Copy the name; set inode_num to indicate new file
        strcpy(directory_table->directory[dt_index].file_name, name);
        directory_table->directory[dt_index].inode_num = NEW_FILE;
    }

    int inode_index = directory_table->directory[dt_index].inode_num;
    //Are we dealing with new file?
    if(inode_index == NEW_FILE){
        //Yes: find inode free in IT; initialize it and change boolean fields to indicate it is occupied
        inode_index = getFreeEntry_IT(i_table);
        if(inode_index == -1) return -1;

        setOccupied_IT(i_table, inode_index);
        init_inode(&i_table->i[inode_index]);
        directory_table->directory[dt_index].inode_num = inode_index;
    }

    //Now, deal with FDT
    int fdt_index = indexInode_FDT(file_descriptor_table, inode_index);
    if(fdt_index == NOT_IN_FDT){
        //Now, the inode we have doesn't have a spot in the FDT; get a new index
        fdt_index = findFreeEntry_FDT(file_descriptor_table);
        if(fdt_index == -1) return -1;

        setOccupied_FDT(file_descriptor_table, fdt_index);

        //Point the FDT entry to the inode
        file_descriptor_table->entries[fdt_index].inode = inode_index;
        //Append ready -> read at beginning, write at end
        file_descriptor_table->entries[fdt_index].rd_ptr = 0;
        file_descriptor_table->entries[fdt_index].wr_ptr = i_table->i[inode_index].size;
    }
    return fdt_index;
}
int sfs_fclose(int fileID){
    //Checks
    if(fileID < 0) return -1; //invalid fileID; can't have negative File Descriptor
    if(!isOccupied_FDT(file_descriptor_table, fileID)) return -1; //invalid fileID; no file in fileID entry

    setUnoccupied_FDT(file_descriptor_table, fileID);
    file_descriptor_table->entries[fileID].inode = 0;
    file_descriptor_table->entries[fileID].rd_ptr = 0;
    file_descriptor_table->entries[fileID].wr_ptr = 0;

    return 0;
}

int sfs_frseek(int fileID, int loc){
    //Checks:
    if(fileID < 0) return -1; //fileID invalid
    if(!isOccupied_FDT(file_descriptor_table, fileID)) return -1; //fileID specifies a location that isn't in use
    if(loc >= MAX_FILE_SIZE) return -1; //if seek is greater than MAX_FILE_SIZE
    if(loc < 0) return -1; //if seek is negative, invalid
    //Grab file descriptor
    FDT_entry* file_descriptor = &file_descriptor_table->entries[fileID];

    //check if loc > size
    if(loc > i_table->i[file_descriptor->inode].size) return -1; //invalid seek: greater than file size

    //if everything is good, shift rd_ptr
    file_descriptor_table->entries[fileID].rd_ptr = loc;
    return 0;
}

int sfs_fwseek(int fileID, int loc){
    //Checks:
    if(fileID < 0) return -1; //fileID invalid
    if(!isOccupied_FDT(file_descriptor_table, fileID)) return -1; //fileID specifies a location that isn't in use
    if(loc >= MAX_FILE_SIZE) return -1; //if seek is greater than MAX_FILE_SIZE
    if(loc < 0) return -1; //if seek is negative, invalid

    //Grab file descriptor
    FDT_entry* file_descriptor = &file_descriptor_table->entries[fileID];

    //check if loc > size
    if(loc > i_table->i[file_descriptor->inode].size) return -1;

    //if everything is good, shift wr_ptr
    file_descriptor_table->entries[fileID].wr_ptr = loc;
    return 0;
}

int sfs_fwrite(int fileID, char *buf, int length){
    //define vars:
    //keep track of bytes to write
    int writtenBytes = 0, bytesLeft = length;
    //indirect pointer block
    indir_ptrs indirectBlock = {};

    //check for valid fileID
    if(!isOccupied_FDT(file_descriptor_table, fileID)) return -1;
    if(length == 0) return -1; //must read something

    //Get FD and inode of desired file
    FDT_entry* file_descriptor = &file_descriptor_table->entries[fileID];
    inode* inode_of_file = &i_table->i[file_descriptor->inode];

    //Setup indirect block
    if(inode_of_file->ind_pointer < 0){
        //Find a block for the pointers
        inode_of_file->ind_pointer = find_free_bit(free_block_map);
        if(inode_of_file->ind_pointer < 0) return -1; //error: not enough space for indirect block
        //mark the bit as used
        set_bit_1(free_block_map, inode_of_file->ind_pointer);
    }
    else read_blocks(DATA_BLOCK_START_INDEX + inode_of_file->ind_pointer, 1, &indirectBlock); //There is already an ind pointer block

    //Now, we have indirect pointer block for the file; let's set up a buffer, to help with moving data
    char* new_buffer = malloc(BLOCKSIZE);

    while(bytesLeft > 0){
        //Set buffer to null
        memset(new_buffer, '\0', BLOCKSIZE);

        int block_index = file_descriptor->wr_ptr / BLOCKSIZE;
        int block_offset = file_descriptor->wr_ptr % BLOCKSIZE;

        int write_amount;
        //A few cases:
        //a) We have more data than the block can provide -> we need to write to fill up the block
        // and the write_amount = BLOCKSIZE - Offset
        //b) We have enough space in the block -> write_amount = bytesLeft
        if(bytesLeft > (BLOCKSIZE - block_offset)) write_amount = BLOCKSIZE - block_offset;
        else write_amount = bytesLeft;

        int newBlock = 0; //flag -> T/F if we are dealing with a new block
        int disk_index;
        if(block_index >= NUM_DIR_PTRS){
            //Thus, we will use an indirect block
            disk_index = indirectBlock.block[block_index - NUM_DIR_PTRS];
            if(disk_index <= 0){
                newBlock = 1;
                disk_index = find_free_bit(free_block_map);
                if(disk_index < 0) break; //disk full
                set_bit_1(free_block_map, disk_index);
                //Save index to indirect block
                indirectBlock.block[block_index - NUM_DIR_PTRS] = disk_index;
            }
        }
        else{
            //direct pointer
            disk_index = inode_of_file->pointer[block_index];
            if(disk_index < 0){
                newBlock = 1;
                //Get new bit
                disk_index = find_free_bit(free_block_map);
                if(disk_index < 0){
                    printf("Disk is full\n");
                    break;
                }
                set_bit_1(free_block_map, disk_index);
                inode_of_file->pointer[block_index] = disk_index;
            }
        }

        if(newBlock == 0){
            //Block is in use, so load the data into new_buffer
            read_blocks(DATA_BLOCK_START_INDEX + disk_index, 1, new_buffer);
        }

        //copy the data into buffer
        memcpy(new_buffer+block_offset, buf, write_amount);

        //write the block to disk
        write_blocks(DATA_BLOCK_START_INDEX + disk_index, 1, new_buffer);

        //Modify the write pointer and end of file indicator (only if write pointer is bigger than end of file
        file_descriptor->wr_ptr += write_amount;
        if(file_descriptor->wr_ptr > inode_of_file->size){
            inode_of_file->size = file_descriptor->wr_ptr;
        }

        //Modify the bytes written and bytes left
        writtenBytes += write_amount;
        bytesLeft -= write_amount;

        //increment buffer so as to remove the portion of text already written
        buf += write_amount;
    }

    //If a ind_pointer was initialized, save the block into disk
    if(inode_of_file->ind_pointer != UNINITIALIZED_POINTER) write_blocks(DATA_BLOCK_START_INDEX + inode_of_file->ind_pointer, 1, &indirectBlock);

    //free data and save state into disk
    free(new_buffer);
    save(free_block_map, sizeof(bitfield), FREE_BIT_MAP_START_INDEX);
    save(i_table, sizeof(inode_table), ITABLE_START_INDEX);
    save(directory_table, sizeof(dir_table), DIRECTORY_BLOCK_START_INDEX);

    return writtenBytes;

}

int sfs_fread(int fileID, char *buf, int length){
    //define vars:
    int bytes_read = 0, remaining_bytes;
    indir_ptrs indirectBlock = {};

    //check for valid fileID
    if(!isOccupied_FDT(file_descriptor_table, fileID)) return -1;

    //Get FD and inode of desired file
    FDT_entry* file_descriptor = &file_descriptor_table->entries[fileID];
    inode* inode_of_file = &i_table->i[file_descriptor->inode];

    //Now, 2 cases:
    //a) length + rd_ptr exceeds the file size
    //b) length + rd_ptr doesn't exceed file size
    if((file_descriptor->rd_ptr + length) > inode_of_file->size) remaining_bytes = inode_of_file->size - file_descriptor->rd_ptr;
    else remaining_bytes = length;

    //indirect block initialization
    if(inode_of_file->ind_pointer != -1) read_blocks(DATA_BLOCK_START_INDEX + inode_of_file->ind_pointer, 1, &indirectBlock); //thus, we have an indirect block -> read this from disk block

    //Allocate empty block buffer
    char* newBuffer = malloc(BLOCKSIZE);

    while(remaining_bytes > 0){
        //Empty the buffer each iteration
        memset(newBuffer, '\0', BLOCKSIZE);

        int block_index = file_descriptor->rd_ptr / BLOCKSIZE;
        int block_offset = file_descriptor->rd_ptr % BLOCKSIZE;

        //2 cases:
        //a)if remaining bytes is more than what's left in the block, read only what's left in the block
        //b)else, read what we need
        int amount_to_read_from_block;
        if(remaining_bytes > (BLOCKSIZE - block_offset)) amount_to_read_from_block = BLOCKSIZE - block_offset;
        else amount_to_read_from_block = remaining_bytes;

        if(block_index >= NUM_DIR_PTRS) read_blocks(DATA_BLOCK_START_INDEX + indirectBlock.block[block_index - NUM_DIR_PTRS], 1, newBuffer);
        else read_blocks(DATA_BLOCK_START_INDEX + inode_of_file->pointer[block_index], 1, newBuffer);

        //Now, we have the block in newBuffer; copy the desired amount (at offset) into buf
        memcpy(buf, newBuffer + block_offset, amount_to_read_from_block);

        //Advance read pointer, bytes_read, buf; decrement remaining_bytes
        file_descriptor->rd_ptr += amount_to_read_from_block;
        bytes_read += amount_to_read_from_block;
        remaining_bytes -= amount_to_read_from_block;

        buf += amount_to_read_from_block; //increment buffer; be sure to decrement at the end
    }
    buf -= bytes_read; //return buf pointer to beginning
    free(newBuffer); //free newBuffer

    return bytes_read; //return bytes_read

}

int sfs_remove(char *file){
    //Checklist:
    //Grab inode -> delete its datablocks
    //Delete inode from inode table
    //Delete directory entry
    //close in file directory table (i.e. set active to no)

    //Grab inode:
    int dir_index = getIndexByName_DT(directory_table, file);

    if(dir_index < 0) return -1; //file not in directory table

    int inode_index = directory_table->directory[dir_index].inode_num;

    //Deal with direct pointers
    int x;
    for(x = 0; x < NUM_DIR_PTRS; x++){
        if(i_table->i[inode_index].pointer[x] != UNINITIALIZED_POINTER){
            //erase block; free bit
            erase_block(i_table->i[inode_index].pointer[x]);
            set_bit_0(free_block_map, i_table->i[inode_index].pointer[x]);
        }
    }
    //Deal with the indirect pointers
    if(i_table->i[inode_index].ind_pointer != UNINITIALIZED_POINTER){
        //Grab the indirect pointer block from disk
        indir_ptrs indirectBlock = {}; //initially get empty indir struct
        read_blocks(i_table->i[inode_index].ind_pointer + DATA_BLOCK_START_INDEX, 1, &indirectBlock);

        //iterate through the indirect pointers
        for(x=0;x<NUM_INDIR_PTRS;x++){
            if(indirectBlock.block[x] > 0 && indirectBlock.block[x] < NUM_BLOCKS){
                //erase block; free bit
                erase_block(indirectBlock.block[x]);
                set_bit_0(free_block_map, indirectBlock.block[x]);
            }
        }

    }
    init_inode(&i_table->i[inode_index]); //reset contents of inode

    //Now, delete inode from inode table, delete dir entry, close in file directory table
    deleteEntry_IT(i_table, inode_index);
    deleteEntry_DT(directory_table, dir_index);

    int fdt_index = indexInode_FDT(file_descriptor_table, inode_index);
    if(fdt_index > -1) sfs_fclose(fdt_index); //it is an entry in FDT, so close

    //now, save reset memory to disk
    save(free_block_map, sizeof(bitfield), FREE_BIT_MAP_START_INDEX);
    save(i_table, sizeof(inode_table), ITABLE_START_INDEX);
    save(directory_table, sizeof(dir_table), DIRECTORY_BLOCK_START_INDEX);

    return 0;
}

void erase_block(int block_index){
    //null will initalize BLOCKSIZE block in memory
    char* null = calloc(BLOCKSIZE,1);
    write_blocks(block_index + DATA_BLOCK_START_INDEX, 1, null);
    free(null);
}





