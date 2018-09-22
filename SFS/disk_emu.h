//
// Created by Sheldon Benard on 2016-11-28.
//

#ifndef PROJECT_DISK_EMU_H
#define PROJECT_DISK_EMU_H

int init_fresh_disk(char *filename, int block_size, int num_blocks);
int init_disk(char *filename, int block_size, int num_blocks);
int read_blocks(int start_address, int nblocks, void *buffer);
int write_blocks(int start_address, int nblocks, void *buffer);
int close_disk();


#endif //PROJECT_DISK_EMU_H
