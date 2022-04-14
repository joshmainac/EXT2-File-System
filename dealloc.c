#include "functions.h"

// Given
// de-allocate an inode (number)<->ialloc

int clr_bit(char *buf, int bit) // clear bit in
{
    // char buf[BLKSIZE];
    buf[bit / 8] &= ~(1 << (bit % 8));
}

int my_idalloc(int dev, int ino)
{
    int i;
    char buf[BLKSIZE];

    if (ino > ninodes)
    {
        printf("inumber %d out of range\n", ino);
        return -1;
    }

    // get inode bitmap block
    get_block(dev, imap, buf);
    clr_bit(buf, ino - 1);

    // write buf back
    put_block(dev, imap, buf);

    // update free inode count in SUPER and GD
    incFreeInodes(dev);
}

// Todo
//  which allocates a free disk block (number) from a device.
// dalloc(int dev)<->balloc(int dev)
int my_bdalloc(int dev, int ino)
{
    int i;
    char buf[BLKSIZE];

    if (ino > ninodes)
    {
        printf("iblk %d out of range\n", ino);
        return -1;
    }

    // get inode bitmap block
    get_block(dev, bmap, buf);
    clr_bit(buf, ino - 1);

    // write buf back
    put_block(dev, bmap, buf);

    // update free inode count in SUPER and GD
    incFreeBlocks(dev);
}