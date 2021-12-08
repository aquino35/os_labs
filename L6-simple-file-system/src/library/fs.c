// fs.cpp: File System

#include "sfs/fs.h"

// #include <algorithm>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Debug file system -----------------------------------------------------------

SuperBlock meta_data;
bool* free_block_bitmap;
Disk * mounted_disk=0;

void debug(Disk *disk) {
    Block block;

    // Read Superblock
    
    disk->readDisk(disk, 0, block.Data);
    
    printf("SuperBlock:\n");
    printf("    magic number is %s\n", (block.Super.MagicNumber == MAGIC_NUMBER ? "valid":"invalid"));
    printf("    %u blocks\n"         , block.Super.Blocks);
    printf("    %u inode blocks\n"   , block.Super.InodeBlocks);
    printf("    %u inodes\n"         , block.Super.Inodes);
   
     // Read Inode blocks
    int numberOfInodeBlocks = block.Super.InodeBlocks;
    for(int inodeBlock=1; inodeBlock<=numberOfInodeBlocks; inodeBlock++){
		disk->readDisk(disk,inodeBlock,block.Data);
		for(unsigned int currentInode=0;currentInode<INODES_PER_BLOCK;currentInode++){
			Inode inode=block.Inodes[currentInode];
			if(inode.Valid==0)
				continue;
			printf("Inode %d:\n",currentInode);
			printf("    size: %u bytes\n",inode.Size);
			printf("    direct blocks:");
			for(unsigned int directBlock=0;directBlock<POINTERS_PER_INODE; directBlock++){
				if(inode.Direct[directBlock]){
					printf(" %u",inode.Direct[directBlock]);
				}
			}
			printf("\n");
			if(inode.Indirect)
            {
				Block indBlock;
				printf("    indirect block: %u\n",inode.Indirect);
				disk->readDisk(disk,inode.Indirect,indBlock.Data);
				printf("    indirect data blocks:");
				for(unsigned int indirectBlock=0; indirectBlock<POINTERS_PER_BLOCK;indirectBlock++){
					if(indBlock.Pointers[indirectBlock])
                    {
						printf(" %u",indBlock.Pointers[indirectBlock]);
					}else
                    {
						break;
					}
				}
				printf("\n");
			}	
		}	
   }
}

// Format file system ----------------------------------------------------------

bool format(Disk *disk) 
{
     if(disk->mounted(disk))
        return false;
    // Write superblock
    Block block;
    block.Super.MagicNumber = MAGIC_NUMBER;
    block.Super.Blocks = disk->size(disk);
    if(disk->size(disk)%10 ==0)
    {
    	block.Super.InodeBlocks=disk->size(disk)/10;
    }else
    {
	block.Super.InodeBlocks=disk->size(disk)/10+1;
    }
    block.Super.Inodes = block.Super.InodeBlocks*INODES_PER_BLOCK;
    disk->writeDisk(disk,0,block.Data);
    // Clear all other blocks
    //char* emptyData = (char *)calloc(BLOCK_SIZE, sizeof(int));
   Block emptyBlock={0};
    for(int i = 1; i < block.Super.Blocks; i++)
    {
        disk->writeDisk(disk,i, emptyBlock.Data);
    }
    return true;
}



// Mount file system -----------------------------------------------------------

bool mount(Disk *disk) 
{
    if(!disk || disk->mounted(disk))
        return false;
    // Read superblock
    Block block;
    disk->readDisk(disk,0, block.Data);
    if(block.Super.MagicNumber != MAGIC_NUMBER || block.Super.Blocks != disk->size(disk))
        return false;
    if(block.Super.Inodes != block.Super.InodeBlocks * INODES_PER_BLOCK)
        return false;
    // Set device and mount
    disk->mount(disk);
     memcpy(&mounted_disk,&disk,sizeof(disk));
    // Copy metadata
    memcpy(&meta_data, &block.Super, sizeof(SuperBlock));
    // Allocate free block bitmap
    int sizeOfBitMap = block.Super.Blocks;
    free_block_bitmap = (bool*) calloc(sizeOfBitMap, sizeof(bool));
    for(unsigned blocks=block.Super.InodeBlocks + 1; blocks < block.Super.Blocks; blocks++)
        free_block_bitmap[blocks] = true;
    Block inodeBlock;
    for(unsigned inodeBlk = 1; inodeBlk <= block.Super.InodeBlocks; inodeBlk++){
        disk->readDisk(disk,inodeBlk, inodeBlock.Data);
        for(unsigned currentInode = 0; currentInode < INODES_PER_BLOCK; currentInode++){
            Inode inode = inodeBlock.Inodes[currentInode];
            if(!inode.Valid) // We have an invalid inode
                continue;
            for(unsigned int directBlock = 0; directBlock < POINTERS_PER_INODE; directBlock++){
                if(inode.Direct[directBlock])
                    free_block_bitmap[inode.Direct[directBlock]] = false;
            }
            if(inode.Indirect)
            {
                free_block_bitmap[inode.Indirect] = false;
                Block indBlock;
                disk->readDisk(disk,inode.Indirect, indBlock.Data);
                for(unsigned int indirectBlock = 0; indirectBlock < POINTERS_PER_BLOCK; indirectBlock++)
                {
                    if(indBlock.Pointers[indirectBlock])
                        free_block_bitmap[indBlock.Pointers[indirectBlock]] = false;
                }
            }
        }
    }
    return true;
}

// Create inode ----------------------------------------------------------------

size_t create()
 {
	   if(!mounted_disk)
       {

		return -1;
        }
        
	    Block block;
	    for(int i = 1; i <= mounted_disk->size(mounted_disk); i++)
        {
	        mounted_disk->readDisk(mounted_disk,i, block.Data);
	        for(int inodes = 0; inodes < INODES_PER_BLOCK; inodes++)
            {
	            // Record inode if found
	            if(!block.Inodes[inodes].Valid)
                {
	                block.Inodes[inodes].Valid = 1;
	                for(unsigned dirPointers = 0; dirPointers < POINTERS_PER_INODE; dirPointers++)
                    {
	                    block.Inodes[inodes].Direct[dirPointers] = 0;
	                }
	                block.Inodes[inodes].Indirect = 0;
	                block.Inodes[inodes].Size = 0;
	                mounted_disk->writeDisk(mounted_disk,i, block.Data);
	                // Array formula. Block * location + offset. Similar to paging
	                return(((i - 1) * INODES_PER_BLOCK) + inodes);
	            }
	        }
	    }
    return -1;
    // Locate free inode in inode table
}

// Remove inode ----------------------------------------------------------------

bool removeInode(size_t inumber) 
{
    // Load inode information
    if(!mounted_disk)
        return -1;
    unsigned inodeBlockNumber = (unsigned)inumber / INODES_PER_BLOCK + 1;
    unsigned offset = inumber % INODES_PER_BLOCK;
    if(inodeBlockNumber > meta_data.InodeBlocks)
        return false;
    Block inodeBlock;
    mounted_disk->readDisk(mounted_disk,inodeBlockNumber, inodeBlock.Data);
    if(!inodeBlock.Inodes[offset].Valid)
        return false;
    inodeBlock.Inodes[offset].Valid = 0;
    inodeBlock.Inodes[offset].Size = 0;
    // Free direct blocks
    for (unsigned dirPointers = 0; dirPointers < POINTERS_PER_INODE; dirPointers++)
    {
        if(!inodeBlock.Inodes[offset].Direct[dirPointers])
            continue;
        free_block_bitmap[inodeBlock.Inodes[offset].Direct[dirPointers]] = true;
        inodeBlock.Inodes[offset].Direct[dirPointers] = 0;
    }
    // Free indirect blocks
    if (inodeBlock.Inodes[offset].Indirect)
    {
        Block indBlock;
        mounted_disk->readDisk(mounted_disk,inodeBlock.Inodes[offset].Indirect, indBlock.Data);
        for (unsigned i = 0; i < POINTERS_PER_BLOCK; i++)
        {
            if (!indBlock.Pointers[i])
                free_block_bitmap[inodeBlock.Pointers[i]] = true;
        }
    }
    free_block_bitmap[inodeBlock.Inodes[offset].Indirect] = true;
    inodeBlock.Inodes[offset].Indirect = 0;
    // Clear inode in inode table
    mounted_disk->writeDisk(mounted_disk,inodeBlockNumber, inodeBlock.Data);
    return true;
}

// Inode stat ------------------------------------------------------------------

size_t stat(size_t inumber) 
{
    // Load inode information
   if(!mounted_disk)
   {
   	return -1;
   }

	unsigned int inodeBlockNumber=(unsigned int)inumber/INODES_PER_BLOCK+1;
	unsigned offset=inumber%INODES_PER_BLOCK;
	if(inodeBlockNumber>meta_data.InodeBlocks)
    {
		return -1;
	}
	Block inodeBlock;
	mounted_disk->readDisk(mounted_disk,inodeBlockNumber,inodeBlock.Data);
	if(!inodeBlock.Inodes[offset].Valid)
    {
		return -1;
	}
	return inodeBlock.Inodes[offset].Size;
}

// Read from inode -------------------------------------------------------------

size_t readInode(size_t inumber, char *data, size_t length, size_t offset) 
{
// Load inode information
    if(!mounted_disk)
    {
        return -1;}
    unsigned inodeBlockNumber = (unsigned)inumber / INODES_PER_BLOCK + 1;
    unsigned inodeOffset = inumber % INODES_PER_BLOCK;
    if(inodeBlockNumber > meta_data.InodeBlocks)
    {
        return -1;
    }
    Block inodeBlock;
    mounted_disk->readDisk(mounted_disk,inodeBlockNumber, inodeBlock.Data);
    Inode inode = inodeBlock.Inodes[inodeOffset];
    if(!inode.Valid || offset > inode.Size)
        return -1;
    size_t blockIndex = offset / BLOCK_SIZE;
    if(blockIndex > (inode.Size / BLOCK_SIZE) + (length % BLOCK_SIZE == 0 ? 0 : 1))
        return -1;
    unsigned writeCount = 0, index = offset;
    Block dataBlock;
    while(writeCount < length && index < inode.Size)
    {
        // Adjust length
        int block = 0;
        // Check if its a dir pointer
        if (blockIndex < POINTERS_PER_INODE)
        {
            block = inode.Direct[blockIndex];
        }
        // Check is indirect exists and get from there
        else if(inode.Indirect)
        {
            if(blockIndex - POINTERS_PER_INODE >= POINTERS_PER_BLOCK)// We have reached the end of the indirect available to us.
                block = -1;
            else
            {
                Block indBlock;
                mounted_disk->readDisk(mounted_disk,inode.Indirect, indBlock.Data);
                block = indBlock.Pointers[blockIndex - POINTERS_PER_INODE];
            }
        }
        // Nothing left on inode to read
        if (block == -1)
            return writeCount;
        // Read block and copy to data
        mounted_disk->readDisk(mounted_disk,block, dataBlock.Data);
        for(unsigned i = (offset % BLOCK_SIZE); i < BLOCK_SIZE && writeCount < length && index < inode.Size; i++)
        {
            data[writeCount++] = dataBlock.Data[i];
            index++;
        }
        // Move to another block since we still need to keep on reading
        offset = 0;
        blockIndex++;
    }
    return writeCount;
}


size_t allocate_free_block() 
{
    // Find an available block if not found return -1
    for(unsigned blocks = meta_data.InodeBlocks + 1; blocks < meta_data.Blocks; blocks++)
    {
        if (free_block_bitmap[blocks])
         {
            free_block_bitmap[blocks] = false;
            return blocks;
        }
    }
    return -1;
}

size_t init_inode_blocks(Inode *inode)
 {
    // Iterate direct pointers
    for (unsigned dirPointer = 0; dirPointer < POINTERS_PER_INODE; dirPointer++) 
    {
        // Verify if there is an empty direct block in inode and init if we can
        if (!inode->Direct[dirPointer]) 
        {
            int block = allocate_free_block();
            if(block == -1)
                return -1;
            inode->Direct[dirPointer] = block;
            return block;
        }
    }
    // We didnt find empty direct block so we check if indirect is available
    if (!inode->Indirect) 
    {
        Block indBlock;
        // Find empty block and assign to indirect
        inode->Indirect = allocate_free_block();
        if (inode->Indirect == -1)
            return -1;
        // Init indirect block
        for (unsigned blkPointer = 1; blkPointer < POINTERS_PER_BLOCK; blkPointer++)
            indBlock.Pointers[blkPointer] = 0;
        // Allocate first block in indirect pointer
        indBlock.Pointers[0] = allocate_free_block();
        if (indBlock.Pointers[0] == -1)
            return -1;
        // Write block
        mounted_disk->writeDisk(mounted_disk,inode->Indirect, indBlock.Data);
        return indBlock.Pointers[0];
    }
    // Read indirect block and go through it to find an empty pointer to fill
    Block indirect;
    mounted_disk->readDisk(mounted_disk,inode->Indirect, indirect.Data);
    for (unsigned block = 1; block < POINTERS_PER_BLOCK; block++) 
    {// Start at one because if indirect exists then at least the first address must be filled
        if (!indirect.Pointers[block]) 
        {
            indirect.Pointers[block] = allocate_free_block();
            if(indirect.Pointers[block] == -1)
                return -1;
            mounted_disk->writeDisk(mounted_disk,inode->Indirect, indirect.Data);
            return indirect.Pointers[block];
        }
    }
    return -1;
}

// Write to inode --------------------------------------------------------------

size_t writeInode(size_t inumber, char *data, size_t length, size_t offset) 
{
    unsigned blockOffset = offset;
    if(!mounted_disk)
    {
        return -1;
    }

    // Get inode block and inode number
    unsigned inodeBlockNumber = (unsigned)inumber / INODES_PER_BLOCK + 1;
    unsigned inodeOffset = inumber % INODES_PER_BLOCK;
    // Check if inode block is after number of inode blocks
    if(inodeBlockNumber > meta_data.InodeBlocks)
        return -1;
    // Load inode
    Block inodeBlock;
    mounted_disk->readDisk(mounted_disk,inodeBlockNumber, inodeBlock.Data);
    Inode *inode = &(inodeBlock.Inodes[inodeOffset]);
    // Verify inode is valid and offset does not over pass size
    if(!inode->Valid || offset > inode->Size)
        return -1;
    size_t blockIndex = offset / BLOCK_SIZE, writeCount=0;
    // Write block and copy to data
    Block dataBlock;
    while (writeCount < length)
     {
        int block = 0;
        // Check if its a dir pointer
        if (blockIndex < POINTERS_PER_INODE)
        {
            block = inode->Direct[blockIndex];
        }
        // Check is indirect exists and get from there
        else if(inode->Indirect)
        {
            if(blockIndex - POINTERS_PER_INODE >= POINTERS_PER_BLOCK)// We have reached the end of the indirect available to us.
                block = -1;
            else
            {
                Block indBlock;
                mounted_disk->readDisk(mounted_disk,inode->Indirect, indBlock.Data);
                block = indBlock.Pointers[blockIndex - POINTERS_PER_INODE];
            }
        }
        // If we didnt find any init blocks so lets init one.
        if(block == 0)
            block = init_inode_blocks(inode);
        // Found none available, we finish writing
        if (block == -1) 
        {
            if(offset + writeCount > inode->Size)
                inode->Size = offset + writeCount;
            mounted_disk->writeDisk(mounted_disk,inodeBlockNumber, inodeBlock.Data);
            return writeCount;
        }
        mounted_disk->readDisk(mounted_disk,block, dataBlock.Data);
        for (unsigned i = blockOffset % BLOCK_SIZE; i < BLOCK_SIZE && writeCount < length; i++)
            dataBlock.Data[i] = data[writeCount++];
        blockOffset = 0;
        mounted_disk->writeDisk(mounted_disk,block, dataBlock.Data);
        blockIndex++;
    }
    if (offset + writeCount > inode->Size)
        inode->Size = offset + writeCount;
    mounted_disk->writeDisk(mounted_disk,inodeBlockNumber, inodeBlock.Data);
    return writeCount;
}