#include "functions.h"

// Given
//  tst_bit, set_bit functions
int tst_bit(char *buf, int bit)
{
    return buf[bit / 8] & (1 << (bit % 8));
}
// Given
int set_bit(char *buf, int bit)
{
    buf[bit / 8] |= (1 << (bit % 8));
}

// Given
int decFreeInodes(int dev)
{
    // dec free inodes count in SUPER and GD
    char buf[BLKSIZE];
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count--;
    put_block(dev, 2, buf);
}

// Given
int incFreeInodes(int dev)
{
    char buf[BLKSIZE];

    // inc free inodes count in SUPER and GD
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_inodes_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (GD *)buf;
    gp->bg_free_inodes_count++;
    put_block(dev, 2, buf);
}

// my-code
int decFreeBlocks(int dev)
{
    char buf[BLKSIZE];
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count--;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (SUPER *)buf;
    gp->bg_free_blocks_count--;
    put_block(dev, 2, buf);
}

// my-code
int incFreeBlocks(int dev)
{
    char buf[BLKSIZE];
    get_block(dev, 1, buf);
    sp = (SUPER *)buf;
    sp->s_free_blocks_count++;
    put_block(dev, 1, buf);

    get_block(dev, 2, buf);
    gp = (SUPER *)buf;
    gp->bg_free_blocks_count++;
    put_block(dev, 2, buf);
}

// Given
//  allocate an inode number from inode_bitmap
int ialloc(int dev)
{
    int i;
    char buf[BLKSIZE];

    // read inode_bitmap block
    get_block(dev, imap, buf);

    for (i = 0; i < ninodes; i++)
    { // use ninodes from SUPER block
        if (tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            put_block(dev, imap, buf);

            decFreeInodes(dev);

            printf("allocated ino = %d\n", i + 1); // bits count from 0; ino from 1
            return i + 1;
        }
    }
    return 0;
}

// Todo
//  which allocates a free disk block (number) from a device.
int balloc(int dev)
{
    int i;
    char buf[BLKSIZE];
    // bmap = gp->bg_block_bitmap;
    get_block(dev, bmap, buf);

    for (i = 0; i < BLKSIZE; i++)
    { // use ninodes from SUPER block
        if (tst_bit(buf, i) == 0)
        {
            set_bit(buf, i);
            put_block(dev, bmap, buf);

            decFreeBlocks(dev);
            // mycode
            memset(buf, 0, BLKSIZE); // buf to 0
            put_block(dev, i + 1, buf);
            //

            // printf("allocated block = %d\n", i + 1); // bits count from 0; ino from 1
            return i + 1;
        }
    }
    return 0;
}
