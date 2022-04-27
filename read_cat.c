#include "functions.h"

int myread(int fd, char *buf, int nbytes)
{
    int i, j, n, offset;
    OFT *oftp;
    MINODE *mip;

    // verify fd
    if (fd < 0 || fd >= NFD)
    {
        // printf("fd %d out of range\n", fd);
        return -1;
    }
    // 1)
    //  OFT's offset number of bytes still available in file.
    oftp = running->fd[fd];
    // verify oftp
    if (oftp == 0)
    {
        // printf("fd %d is not open\n", fd);
        return -1;
    }
    // verify mode
    //  if (oftp->mode != 0 && oftp->mode != 2)
    //  {
    //      printf("fd %d is not in read mode\n", fd);
    //      return -1;
    //  }
    mip = oftp->minodePtr;
    // number of bytes still available in file.
    int avil = mip->INODE.i_size - oftp->offset;

    // cq points at buf[ ]
    char *cq = buf;

    // verifing input
    if (mip->INODE.i_size < oftp->offset)
    {
        // printf("fd %d is not open for reading\n", fd);
        return -1;
    }
    if (nbytes < 0)
    {
        // printf("nbytes < 0\n");
        return -1;
    }
    if (oftp->offset + nbytes > mip->INODE.i_size)
    {
        nbytes = mip->INODE.i_size - oftp->offset;
    }
    if (nbytes == 0)
    {
        // printf("nbytes == 0\n");
        return 0;
    }
    //

    offset = oftp->offset;
    int count = 0;
    while (nbytes && avil)
    {
        // Compute LOGICAL BLOCK number lbk and  startByte in that block from offset;
        int lbk = oftp->offset / BLKSIZE;
        int startByte = oftp->offset % BLKSIZE;
        int blk;
        char readbuf[1024];
        int *np;
        int indirect_blk;
        int indirect_off;
        // I only show how to read DIRECT BLOCKS. YOU do INDIRECT and D_INDIRECT
        // ink is num of blocks
        if (lbk < 12)
        {
            // lbk is a direct block
            // map LOGICAL lbk to PHYSICAL blk
            blk = mip->INODE.i_block[lbk];
        }
        else if (lbk >= 12 && lbk < 256 + 12)
        {
            //  indirect blocks
            // direct (12),indirect(256),lbk(total)
            get_block(mip->dev, mip->INODE.i_block[12], readbuf);
            // add the total number of blocks - direct blocks
            np = (int *)readbuf + lbk - 12;
            blk = *np;
        }
        else
        {
            //  double indirect blocks
            //  indirect blocks
            // direct (12),indirect(256),lbk(total)
            get_block(mip->dev, mip->INODE.i_block[13], readbuf);
            // num of idb in dind
            // lbk is total blocks. subtract i_blocks ~[12]
            // number of indirect blocks used in my double indirect block
            indirect_blk = (lbk - 256 - 12) / 256;
            np = (int *)readbuf + indirect_blk;
            indirect_off = (lbk - 256 - 12) % 256;
            get_block(mip->dev, *np, readbuf);
            np = (int *)readbuf + indirect_off;
            blk = *np;

            // get_block(mip->dev, mip->INODE.i_block[13], readbuf);

            // indirect_blk = (lbk - 256 - 12) / 256;
            // indirect_off = (lbk - 256 - 12) % 256;
            // printf("blk = %d, ofset = %d\n", indirect_blk, indirect_off);
            // // getchar();

            // ip = (int *)readbuf + indirect_blk;
            // // getchar();
            // get_block(mip->dev, *np, readbuf);
            // // etchar();
            // ip = (int *)readbuf + indirect_off;
            // blk = *np;
            // // getchar();

            //  get_block(mip->dev, mip->INODE.i_block[13], readbuf);
            //  np = (int *)readbuf + (lbk - (12 + 256));
            //  indirect_blk = *np;
            //  get_block(mip->dev, indirect_blk, readbuf);
            //  np = (int *)readbuf + (lbk - (12 + 256 + 256));
            //  indirect_off = *np;
            //  get_block(mip->dev, indirect_off, readbuf);
            //  blk = *(int *)readbuf;

            // get_block(mip->dev, mip->INODE.i_block[13], readbuf);
            // np = (int *)readbuf + lbk - (12 + 256);
            // blk = *np;

            // get_block(mip->dev, mip->INODE.i_block[13], readbuf);

            // indirect_blk = (lbk - 256 - 12) / 256;
            // indirect_off = (lbk - 256 - 12) % 256;
            // printf("blk = %d, ofset = %d\n", indirect_blk, indirect_off);
            // getchar();

            // np = (int *)readbuf + indirect_blk;
            // getchar();
            // get_block(mip->dev, *np, readbuf);
            // getchar();
            // np = (int *)readbuf + indirect_off;
            // blk = *np;
            // getchar();
            // printf("double indirect\n");
        }

        /* get the data block into readbuf[BLKSIZE] */

        get_block(mip->dev, blk, readbuf);
        /* copy from startByte to buf[ ], at most remain bytes in this block */
        // int startByte = oftp->offset % BLKSIZE;->offset that does not fill a block
        char *cp = readbuf + startByte;
        int remain = BLKSIZE - startByte;
        // char *cq = buf;
        // int myread(int fd, char *buf, int nbytes)
        // read data into buf
        while (remain > 0)
        {
            // printf("cp1 = %c\n", *cp);
            *cq++ = *cp++;  // copy byte from readbuf[] into buf[]
            oftp->offset++; // advance offset
            count++;        // inc count as number of bytes read
            avil--;
            nbytes--;
            remain--;
            // printf("cp2q = %s\n", readbuf);
            // printf("cp3 = %c\n", *cp);
            //   printf("cp = %s\n", readbuf);
            if (nbytes <= 0 || avil <= 0)
                break;
        }
        // if one data block is not enough, loop back to OUTER while for more ...
    }
    // printf("myread: read %d char from file descriptor %d\n", count, fd);

    return count; // count is the actual number of bytes read
}

// read [file_name] [nbytes]
int read_file()
{
    char cmd[256], my_file[256], my_nbytes[256]; // get this from line
    sscanf(line, "%s %s %s", cmd, my_file, my_nbytes);
    printf("cmd=%s my_file=%s my_nbytes=%s\n", cmd, my_file, my_nbytes);
    int fd = atoi(my_file);
    int nbytes = atoi(my_nbytes);

    // char buf[nbytes + 1];
    char buf[BLKSIZE];
    char readbuf[BLKSIZE];
    // ASSUME: file is opened for RD or RW;
    // ask for a fd  and  nbytes to read;
    // verify that fd is indeed opened for RD or RW;

    int actual = myread(fd, buf, BLKSIZE);

    if (actual == -1)
    {
        printf("myread() failed\n");
        return 0;
    }

    memcpy(readbuf, buf, nbytes);
    readbuf[nbytes] = '\0';

    printf("actual = %d \n", actual);
    printf("********* read file %d %d ********* \n", fd, nbytes);
    printf("%s\n", readbuf);
    printf("***********************************\n");
    printf("actual # of char read = %d\n", actual);
    lseek_file();
    return actual;
}

// mod this
//  cat filename:
int cat_file()
{
    char cmd[256], my_file[256]; // get this from line
    sscanf(line, "%s %s", cmd, my_file);
    printf("cmd=%s my_file=%s\n", cmd, my_file);
    int fd = atoi(my_file);
    char mybuf[BLKSIZE];
    // ASSUME: file is opened for RD or RW;
    // ask for a fd  and  nbytes to read;
    // verify that fd is indeed opened for RD or RW;
    int n = 0;
    // open
    myopen(my_file, 0);

    // read
    n = myread(fd, mybuf, BLKSIZE);
    printf("\n");
    printf("\n");
    printf("********* cat file %d %d ********* ********* ********* *********\n", fd, BLKSIZE);
    int count = n;
    while (n)
    {

        mybuf[n] = 0; // as a null terminated string

        // printf("%s", mybuf);   <=== THIS works but not good

        printf("%s", mybuf);
        // spit out chars from mybuf[] but handle \n properly;
        n = myread(fd, mybuf, BLKSIZE);
        count = count + n;
    }
    printf("\n");
    printf("*********************************** ********* ********* *********\n");
    printf("actual # of char read = %d\n", count);
    printf("\n");
    printf("\n");

    // nis actual read char
    // n = 0;
    // int nn = 0;
    // while (n = myread(fd, mybuf[1024], 1024))
    // {
    //     nn += n;
    //     mybuf[nn] = 0;       // as a null terminated string
    //     printf("%s", mybuf); //<=== THIS works but not good
    //     // spit out chars from mybuf[] but handle \n properly;
    // }

    // close
    close_file();
    return n;
}
