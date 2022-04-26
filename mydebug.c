#include "functions.h"

int mount_table()
{
    int i;
    for (i = 0; i < 8; i++)
    {
        if (mountTable[i].dev != 0)
        {
            MINODE *mymip = mountTable[i].mounted_inode;
            printf("mountTable[%d]\n", i);
            printf("%s mounted on [%d,%d]\n", mountTable[i].name, mymip->dev, mymip->ino);
            printf("dev=%d ninodes=%d nblocks=%d bmap=%d imap=%d iblk=%d\n", mountTable[i].dev, mountTable[i].ninodes, mountTable[i].nblocks, mountTable[i].bmap, mountTable[i].imap, mountTable[i].iblk);
            printf("mounted_inode=[%d,%d]\n", mymip->dev, mymip->ino);
            printf("name=%s\n", mountTable[i].name);
            printf("mount_name=%s\n", mountTable[i].mount_name);
        }
        else
        {
            printf("mountTable[%d] is FREE\n", i);
        }
    }
}

int my_mip()
{

    int i;
    MINODE *mip;
    char buf[BLKSIZE];
    int blk, offset;
    INODE *ip;

    // go through minode array
    // if ref count not 0 & dev & ino match
    // increase ref count and return that inode
    // for (i = 0; i < NMINODE; i++)
    for (i = 0; i < 10; i++)
    {
        mip = &minode[i];
        printf("(%d)minode=[%d,%d]->mounted=%d->refcount=%d\n", i, mip->dev, mip->ino, mip->mounted, mip->refCount);
    }

    return 0;
}

int my_dev()
{

    MINODE *wd = running->cwd;
    printf("my MINODE= [%d.%d]\n", dev, wd->ino);

    return 0;
}
