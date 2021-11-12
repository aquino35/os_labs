// fs.cpp: File System

#include "../../include/sfs/fs.h"

// #include <algorithm>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Debug file system -----------------------------------------------------------

void debug(Disk *disk) {

    Block block;

    // Read Superblock
    disk->readDisk(disk, 0, block.Data);
    
    printf("SuperBlock:\n");
    printf("    %u blocks\n"         , block.Super.Blocks);
    printf("    %u inode blocks\n"   , block.Super.InodeBlocks);
    printf("    %u inodes\n"         , block.Super.Inodes);
    
    
    // Read Inode blocks
    Block inode_block;
    // Loop through inode blocks
    for (uint32_t i = 1; i <= block.Super.InodeBlocks; i++) 
    { 
        
        disk->readInode( i, inode_block.Data);

        // Loop through inodes
        for (uint32_t j = 0; j < INODES_PER_BLOCK; j++) 
        {  
            Inode inode = inode_block.Inodes[j];
            if (inode.Valid) 
            {
                printf("\nInode %d:\n", j);
                printf("    size: %u bytes\n", inode.Size);
                printf("    direct blocks:");

                // Loop through direct pointers
                for (uint32_t k = 0; k < POINTERS_PER_INODE; k++) 
                { 
                    if (inode.Direct[k]) 
                    {
                        printf(" %lu", (unsigned long)(inode.Direct[k]));
                    }
                }
                if (inode.Indirect) 
                {  
                    printf("\n    indirect block: %lu\n", (unsigned long)(inode.Indirect));
                    printf("    indirect data blocks:");

                    Block indirect_block;
                    disk->readInode(inode.Indirect, indirect_block.Data);
                    // Loop through indirect pointers
                    for (uint32_t l = 0; l < POINTERS_PER_BLOCK; l++) 
                    {
                        if (indirect_block.Pointers[l])
                        {
                            printf(" %d",(indirect_block.Pointers[l]));
                        }
                    }
                } 
            }
        }
    }
    
}

// Format file system ----------------------------------------------------------

bool format(Disk *disk) {
    
    // Write superblock
    
    // Clear all other blocks
    
    return true;
}



// Mount file system -----------------------------------------------------------

bool mount(Disk *disk) {
    // Read superblock

    // Set device and mount

    // Copy metadata

    // Allocate free block bitmap

    return true;
}

// Create inode ----------------------------------------------------------------

size_t create() {
    // Locate free inode in inode table

    // Record inode if found
    return 0;
}

// Remove inode ----------------------------------------------------------------

bool removeInode(size_t inumber) {
    // Load inode information

    // Free direct blocks

    // Free indirect blocks

    // Clear inode in inode table
    return true;
}

// Inode stat ------------------------------------------------------------------

size_t stat(size_t inumber) {
    // Load inode information
    return 0;
}

// Read from inode -------------------------------------------------------------

size_t readInode(size_t inumber, char *data, size_t length, size_t offset) {
    if (inumber >= this->inodes){ 
        return -1;
    }

    // Load inode information
    Inode loadedInode;
    bool validInode = load_inode(inumber, &loadedInode);
    if (!validInode) { 
        return -1; 
    }

    if (loadedInode.Size == offset) { 
        return -1; 
    }

    // Adjust length
    length = std::min(length, loadedInode.Size - offset);

    uint32_t startBlock = offset/Disk::BLOCK_SIZE;
    uint32_t startByte  = offset%Disk::BLOCK_SIZE;
   
    // Read block and copy to data
    std::string dataString = "";
    Block readFromBlock;
    size_t   readIndex = length;
    uint32_t dataIndex = 0;
    while (startBlock < POINTERS_PER_INODE ){
        if (!loadedInode.Direct[startBlock]){
            startBlock++;
            startByte = 0;
            continue;
        }
        
        disk->read(loadedInode.Direct[startBlock], readFromBlock.Data);
        size_t blockSizeVar = Disk::BLOCK_SIZE;
        uint32_t dataSize = std::min(startByte + readIndex, blockSizeVar);
        uint32_t incrementer = startByte;
        
        while (incrementer < dataSize){
            dataString += readFromBlock.Data[incrementer];
            incrementer++;
            dataIndex++;
        }
        readIndex = readIndex - dataSize + startByte;
        startByte = 0;
        startBlock++;
    }       

    Block indirectBlock;
    if (readIndex && loadedInode.Indirect){
        disk->read(loadedInode.Indirect, indirectBlock.Data);
        startBlock = startBlock - POINTERS_PER_INODE;
        while (startBlock < POINTERS_PER_BLOCK){
            if (!indirectBlock.Pointers[startBlock]){
                startBlock++;
                startByte = 0;
                continue;       
            }
            disk->read(indirectBlock.Pointers[startBlock], readFromBlock.Data);
            size_t blockSizeVar = this->disk->BLOCK_SIZE;
            uint32_t dataSize = std::min(startByte + readIndex, blockSizeVar);
            uint32_t incrementer = startByte;
            while (incrementer < dataSize){
                dataString += readFromBlock.Data[incrementer];
                dataIndex++;
                incrementer++;
            }
            readIndex = readIndex - dataSize + startByte;
            startByte = 0;
            startBlock++;
            
        }
    }

    memcpy(data, dataString.c_str(), dataString.size());
    return dataIndex;
}

// Write to inode --------------------------------------------------------------

size_t writeInode(size_t inumber, char *data, size_t length, size_t offset) {
    // Load inode
    
    // Write block and copy to data
    return 0;
}
