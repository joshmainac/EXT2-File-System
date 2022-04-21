#include "functions.h"

int mywrite(int fd, char buf[], int nbytes)
{
    int i, j, n, offset;
    OFT *oftp;
    oftp = running->fd[fd];
    MINODE *mip;
    mip = oftp->minodePtr;
    int lbk, startByte, blk, dblk;
    char wbuf[BLKSIZE];
    int ibuf[256], dbuf[256];
    char *cp;
    int remain;
    char *cq = buf;
    int count = 0;
    while (nbytes > 0)
    {
        // printf("nbytes = %d\n", nbytes);

        // compute LOGICAL BLOCK(lbk) and the startByte in that lbk :

        // data size,offset
        lbk = oftp->offset / BLKSIZE;
        // remain is startByte
        startByte = oftp->offset % BLKSIZE;

        // I only show how to write DIRECT data blocks, you figure out how to
        // write indirect and double-indirect blocks.
        // printf("lbk = %d, startByte = %d\n", lbk, startByte);
        if (lbk < 12)
        { // direct block
          // printf("db\n");
            if (mip->INODE.i_block[lbk] == 0)
            { // if no data block yet

                mip->INODE.i_block[lbk] = balloc(mip->dev); // MUST ALLOCATE a block
            }
            // mip = oftp->minodePtr;
            blk = mip->INODE.i_block[lbk]; // blk should be a disk block now
        }
        else if (lbk >= 12 && lbk < 256 + 12)
        { // INDIRECT blocks
            // HELP INFO:
            // printf("idb\n");
            if (mip->INODE.i_block[12] == 0)
            {
                //   allocate a block for it;
                //   zero out the block on disk !!!!
                mip->INODE.i_block[12] = balloc(mip->dev);
                memset(ibuf, 0, 256);
            }
            // get i_block[12] into an int ibuf[256];
            get_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf);
            // blk = ibuf[lbk - 12];
            blk = ibuf[lbk - 12];
            if (blk == 0)
            {
                // allocate a disk block;
                mip->INODE.i_block[lbk] = balloc(mip->dev);
                // record it in i_block[12];
                ibuf[lbk - 12] = mip->INODE.i_block[lbk];
            }
            //.......
        }
        else
        {
            // double indirect blocks */
            memset(ibuf, 0, 256);
            get_block(mip->dev, mip->INODE.i_block[13], (char *)dbuf);
            lbk -= (12 + 256);
            dblk = dbuf[lbk / 256];
            get_block(mip->dev, dblk, (char *)dbuf);
            blk = dbuf[lbk % 256];
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////

        // int blk the datablock(offset)
        // wbuf is the current data block, reads current text

        // Now we have blk, the disk block to contain the data.
        /* all cases come to here : write to the data block */
        memset(wbuf, 0, BLKSIZE);
        get_block(mip->dev, blk, wbuf); // read disk block into wbuf[ ]

        // reset
        // memset(wbuf, 0, BLKSIZE);
        // oftp->minodePtr->INODE.i_size = 0;

        cp = wbuf + startByte;        // cp points at startByte in wbuf[]
        remain = BLKSIZE - startByte; // number of BYTEs remain in this block
        // printf("remain = %d\n", remain);

        // Don't know but whiel loop ti if and it works
        if (remain > 0)
        {
            // printf("remain = %d\n", mip->INODE.i_size);
            count++;
            // write as much as remain allows
            // cq points at buf[ ], char *cq = buf;
            // cq is the text we want to write, add this to cp
            *cp++ = *cq++;
            nbytes--;
            remain--;       // dec counts
            oftp->offset++; // advance offset
            if (oftp->offset > mip->INODE.i_size)
            {
                // especially for RW|APPEND mode
                mip->INODE.i_size++; // inc file size (if offset > fileSize)
            }

            if (nbytes <= 0)
                break; // if already nbytes, break
        }
        // cp points at wbuf startBytes, we add cq to cp so wbuf is changed
        put_block(mip->dev, blk, wbuf); // write wbuf[ ] to disk

        // loop back to outer while to write more .... until nbytes are written
    }

    mip->dirty = 1; // mark mip dirty for iput()
    // printf("wrote %d char into file descriptor fd=%d\n", count, fd);
    return count;
}

// write [fd] [text]
int write_file()
{
    // 1. Preprations: ask for a fd   and   a text string to write;
    char cmd[256], my_file[256], my_text[256]; // get this from line
    sscanf(line, "%s %s %s", cmd, my_file, my_text);
    printf("cmd=%s my_file=%s my_nbytes=%s\n", cmd, my_file, my_text);
    int fd = atoi(my_file);
    // int nbytes = atoi(my_text);

    // 2. verify fd is indeed opened for WR or RW or APPEND mode

    // 3. copy the text string into a buf[] and get its length as nbytes.
    return (mywrite(fd, my_text, strlen(my_text)));
}

// cp src dest:
// int my_cp_0()
// {
//     char cmd[256], src[256], dest[256];
//     sscanf(line, "%s %s %s", cmd, src, dest);
//     printf("cmd=%s src=%s dest=%s\n", cmd, src, dest);
//     // change opeen to original code
//     // not resetting
//     int src_fd = myopen(src, 0);
//     int dest_fd = myopen(dest, 1);
//     char buf[BLKSIZE];
//     int nbytes;
//     // while ((nbytes = myread(src_fd, buf, BLKSIZE)) > 0)
//     // {
//     //     mywrite(dest_fd, buf, nbytes);
//     // }
//     nbytes = myread(src_fd, buf, BLKSIZE);
//     buf[nbytes] = '\0';
//     printf("%s\n", buf);
//     mywrite(dest_fd, buf, nbytes);

//     myclose(src_fd);
//     myclose(dest_fd);
//     return 0;
// }

int my_cp()
{
    char cmd[256], src[256], dest[256];
    sscanf(line, "%s %s %s", cmd, src, dest);
    printf("cmd=%s src=%s dest=%s\n", cmd, src, dest);

    int ino = getino(dest);
    if (ino == 0)
    {
        printf("file %s does not exist\n", dest);
        MINODE *wd = running->cwd;
        int pino = search(wd, ".");
        MINODE *pmip = iget(dev, pino);
        my_kcreat(pmip, dest);
    }
    // change opeen to original code
    // not resetting
    int src_fd = myopen(src, 0);
    int dest_fd = myopen(dest, 1);
    char buf[BLKSIZE];
    int nbytes;
    int count = 0;
    while ((nbytes = myread(src_fd, buf, BLKSIZE)) > 0)
    {
        count++;
        // printf("cp count = %d\n", count);
        mywrite(dest_fd, buf, nbytes);
    }

    myclose(src_fd);
    myclose(dest_fd);
    return 0;
}

// 1. verify src exists; get its INODE in ==> you already know its dev
// 2. check whether src is on the same dev as src
//               CASE 1: same dev:
// 3. Hard link dst with src (i.e. same INODE number)
// 4. unlink src (i.e. rm src name from its parent directory and reduce INODE's
//                link count by 1).
//               CASE 2: not the same dev:
// 3. cp src to dst
// 4. unlink src
// mv src dest:
int my_mv()
{
    char cmd[256], src[256], dest[256];
    sscanf(line, "%s %s %s", cmd, src, dest);
    printf("cmd=%s src=%s dest=%s\n", cmd, src, dest);
    // 1. verify src exists; get its INODE in
    int ino = getino(src);
    if (ino == 0)
    {
        printf("file %s does not exist\n", src);
        return -1;
    }
    MINODE *src_mip = iget(dev, ino);

    // 2. check whether src is on the same dev as src
    if (src_mip->dev == dev)
    {
        printf("same dev\n");
        // CASE 1: same dev:
        //  3. Hard link dst with src
        my_link();
        // 4. unlink src
        my_unlink();
    }
    else
    {
        // CASE 2: not the same dev:
        printf("not the same dev\n");
        // 3. cp src to dst
        my_cp();
        // 4. unlink src
        my_unlink();
    }

    iput(src_mip);
    return;
}
// cp tiny file1
