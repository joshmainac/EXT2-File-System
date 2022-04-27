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
    int ibuf[256], dbuf[256], iibuf[256];
    char *cp;
    int remain;
    char *cq = buf;
    int count = 0;
    int indirect_lbk = 0;
    while (nbytes > 0)
    {

        lbk = oftp->offset / BLKSIZE;
        startByte = oftp->offset % BLKSIZE;

        if (lbk < 12)
        {
            // direct block
            // 1) access the iblock and simply allocate a block

            if (mip->INODE.i_block[lbk] == 0)
            {

                mip->INODE.i_block[lbk] = balloc(mip->dev); // MUST ALLOCATE a block
            }
            // mip = oftp->minodePtr;
            blk = mip->INODE.i_block[lbk]; // blk should be a disk block now
        }
        else if (lbk >= 12 && lbk < 256 + 12)
        {
            // single indirect block
            // 1)access i_block[12] and allocate a block
            // 2) i_block[12] is a indirect block that points to 256 disk blocks
            // 3)initialize these disk blocks to 0
            // 4) if i_block[12] != 0, skip 1) ~ 3)

            // 5) get_block(i_block[12]) and store it in ibuf[]
            // 6)access the end of ibuf[iblk] adn allocate disk block]=

            if (mip->INODE.i_block[12] == 0)
            {
                mip->INODE.i_block[12] = balloc(mip->dev); // i_[12]->indirect block 256 entry
                get_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf);
                memset(ibuf, 0, 256); // assign the entry to zero
                // put_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf);
            }

            // get i_block[12] into an int ibuf[256];
            get_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf);
            // blk - 12 --> total blocks used
            blk = ibuf[lbk - 12]; // check if empty
            if (blk == 0)
            {
                // allocate a disk block;
                // NO?-->mip->INODE.i_block[lbk] = balloc(mip->dev);
                // int blk = allocated block size
                blk = balloc(mip->dev);
                // write blk into ibuf[lbk-12]
                // lbk = total indirect blocks used
                // assign blk at the end of ibuf[lbk-12]
                ibuf[lbk - 12] = blk;

                // record it in i_block[12];
                // NO?-->ibuf[lbk - 12] = mip->INODE.i_block[lbk];
                put_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf);
                get_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf);
            }
            //.......
            get_block(mip->dev, mip->INODE.i_block[12], (char *)ibuf); // get i_block[13] into dbuf
            indirect_lbk = lbk - 12;                                   // lbk = total indirect blocks used
            blk = ibuf[indirect_lbk];                                  // each block in i_block[13] is 256 entry
        }
        else
        {
            // double indirect blocks */
            printf("double indirect blocks-1\n");

            // if i_block[13] is empty, allocate a block
            if (mip->INODE.i_block[13] == 0)
            {
                mip->INODE.i_block[13] = balloc(mip->dev);
                get_block(mip->dev, mip->INODE.i_block[13], (char *)dbuf);
                memset(ibuf, 0, 256); // assign the entry to zero
                // put_block(mip->dev, mip->INODE.i_block[13], (char *)dbuf);
                for (int jj = 0; jj < 256; jj++)
                {
                    // make one block and initialize it
                    blk = balloc(mip->dev);
                    get_block(mip->dev, blk, (char *)ibuf);
                    memset(ibuf, 0, 256);
                    //
                    dbuf[jj] = blk; // assign new block to dbuf
                }
            }

            // access the dbuf
            get_block(mip->dev, mip->INODE.i_block[13], (char *)dbuf);
            indirect_lbk = lbk - 12 - 256;
            dblk = dbuf[indirect_lbk / 256]; // this will be zero

            get_block(mip->dev, dblk, (char *)ibuf); // get ibuf
            blk = ibuf[indirect_lbk % 256];          // access one entry in ibuf, it's zero

            if (blk == 0)
            {
                // allocate a disk block;
                // NO?-->mip->INODE.i_block[lbk] = balloc(mip->dev);
                // int blk = allocated block size
                blk = balloc(mip->dev);
                // write blk into ibuf[lbk-12]
                // lbk = total indirect blocks used
                // assign blk at the end of ibuf[lbk-12]
                ibuf[indirect_lbk % 256] = blk; // now it points to block

                // record it in i_block[12];
                // NO?-->ibuf[lbk - 12] = mip->INODE.i_block[lbk];
                put_block(mip->dev, dblk, (char *)ibuf);
                get_block(mip->dev, dblk, (char *)ibuf);
            }
            get_block(mip->dev, dblk, (char *)ibuf);
            blk = ibuf[indirect_lbk % 256];

            // // initiallize block as ibuf
            // memset(ibuf, 0, 256);
            // put_block(mip->dev, dblk, (char *)ibuf); // ibuf is initialize
            // if (dblk == 0)
            // {
            //     // initialize dbuf
            //     //  put 1 block in 1st dbuf(mod dbuf)
            //     dblk = balloc(mip->dev);         // dblk is the address of the block
            //     dbuf[indirect_lbk / 256] = dblk; // now this points at teh address of the block
            //     put_block(mip->dev, mip->INODE.i_block[13], (char *)dbuf);
            // }

            // // get dbuf
            // get_block(mip->dev, mip->INODE.i_block[13], (char *)dbuf); // get i_block[13] into dbuf
            // // get the address of the block in dbuf
            // dblk = dbuf[indirect_lbk / 256]; // each block in i_block[13] is 256 entry
            // // get ibuf
            // get_block(mip->dev, dblk, (char *)ibuf); // get the last block in i_block[13]
            // // access teh addree of the block in ibuf

            // blk = ibuf[indirect_lbk % 256]; // the move the indirect_offset
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
        // printf("cp = %s\n", wbuf);
        // Don't know but whiel loop ti if and it works
        while (remain > 0)
        {
            // printf("startbyte = %d\n", startByte);
            // printf("size = %d\n", mip->INODE.i_size);

            count++;
            // write as much as remain allows
            // cq points at buf[ ], char *cq = buf;
            // cq is the text we want to write, add this to cp
            *cp++ = *cq++;
            // printf("cp = %s\n", wbuf);

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
    printf("wrote %d char into file descriptor fd=%d lbk=%d\n", count, fd, lbk);
    return count;
}

// write [fd] [text]
int write_file()
{
    // 1. Preprations: ask for a fd   and   a text string to write;
    char cmd[256], my_file[256], my_text[256]; // get this from line
    sscanf(line, "%s %s %s", cmd, my_file, my_text);
    printf("cmd=%s my_file=%s my_nbytes=%d\n", cmd, my_file, strlen(my_text));
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
    while ((nbytes = myread(src_fd, buf, BLKSIZE)))
    {
        count++;
        // printf("cp count = %d\n", count);
        // buf[nbytes] = '\0'; // as a null terminated string
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
