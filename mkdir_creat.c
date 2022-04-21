#include "functions.h"

// pmip->write below this,ino->enter this ino, name-> this name
int enter_name(MINODE *pmip, int ino, char *name)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  enter_name()\n");
    char sbuf[BLKSIZE], temp[256];
    char *cp;
    DIR *dp;
    INODE *pip = &pmip->INODE;

    printf("entername-1\n");
    // for each data block of parent DIR do // assume: only 12 direct blocks
    for (int i = 0; i < 12; i++)
    {
        printf("entername-1A\n");

        if (pip->i_block[i] == 0)
            break;

        printf("entername-1B\n");

        //(1). Get parent's data block into a buf[ ];
        get_block(dev, pmip->INODE.i_block[0], sbuf);
        dp = (DIR *)sbuf;
        cp = sbuf;
        printf("entername-2\n");
        //
        // int name_len = strlen(name);
        // int ideal_length = 4 * ((8 + name_len + 3) / 4); // a multiple of 4
        //

        dp = (DIR *)sbuf;
        cp = sbuf;
        while (cp + dp->rec_len < sbuf + BLKSIZE)
        {
            cp += dp->rec_len;
            dp = (DIR *)cp;
        }

        //////??????
        // dp is dir3

        // ideal length for the current last dir
        int ideal_length = 4 * ((8 + dp->name_len + 3) / 4); // a multiple of 4

        printf("inode %d\n", dp->inode);
        printf("rec_len %d\n", dp->rec_len);
        printf("dp->name_len %d\n", dp->name_len);
        // printf("temp[dp->name_len] %s\n", temp);
        // printf("name_len = strlen(temp)%d\n", name_len);

        // n_len is the name length of new entry
        int n_len = strlen(name);
        int need_length = 4 * ((8 + n_len + 3) / 4); // a multiple of 4

        int remain = dp->rec_len - ideal_length;

        // dp NOW points at last entry in block remain = LAST entry's rec_len - its ideal_length;
        // dp is dir3
        if (remain >= need_length)
        {
            printf("entername-3\n");
            // enter the new entry as the LAST entry and
            //     trim the previous entry rec_len to its
            //         ideal_length;

            // change dir3 rec_len
            dp->rec_len = ideal_length;
            // move
            cp += dp->rec_len;
            dp = (DIR *)cp;
            //
            dp->inode = ino;
            strcpy(dp->name, name);
            dp->name_len = n_len;
            dp->rec_len = remain;
        }

        //////??????
        // goto step (6);
        printf("entername-4\n");
        put_block(dev, pmip->INODE.i_block[0], sbuf);
    }

    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
}

// pip is parent inode
int my_kmkdir(MINODE *pip, char child[256])
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  my_kmdir()\n");
    //(1)
    // Allocate an INODE and a disk block
    int ino = ialloc(dev);

    int blk = balloc(dev);

    //(2)
    // load INODE into a minode
    MINODE *mip = iget(dev, ino);
    INODE *ip = &mip->INODE;

    // initialize mip->INODE as a DIR INODE;
    ip->i_mode = 0x41ED;

    ip->i_uid = running->uid; // owner uid
    ip->i_gid = running->gid; // group Id
    ip->i_size = BLKSIZE;     // size in bytes
    ip->i_links_count = 2;    // links count=2 because of . and ..
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 2; // LINUX: Blocks count in 512-byte chunks
    // mip->INODE.i_block[0] = blk;
    ip->i_block[0] = blk; // new DIR has one datablock
                          // other i_block[ ] = 0;

    for (int i = 1; i < 15; i++)
    {
        mip->INODE.i_block[i] = 0;
    }
    // mark minode modified (dirty);
    mip->dirty = 1;
    // write INODE back to disk

    iput(mip);

    //(3)
    // make data block 0 of INODE to contain . and .. entries;
    // write to disk block blk.
    char buf[BLKSIZE];
    bzero(buf, BLKSIZE); // optional: clear buf[ ] to 0 DIR *dp = (DIR *)buf;
    // make . entry
    DIR *dp = (DIR *)buf;

    // ino int ino = ialloc(dev);
    dp->inode = ino;

    dp->rec_len = 12;

    dp->name_len = 1;

    dp->name[0] = '.';

    // make .. entry: pino=parent DIR ino,
    // blk = allocated block!!!!!!!!!!!!!!!!!!!!!!!!!!!

    dp = (char *)dp + 12;
    dp->inode = pip->ino;
    dp->rec_len = BLKSIZE - 12; // rec_len spans block
    dp->name_len = 2;
    dp->name[0] = dp->name[1] = '.';
    put_block(dev, blk, buf); // write to blk on  disk

    //(4)
    // pip->int my_kmkdir(MINODE *pip, char child[256]),ino->inode for new entry,new entry name.
    enter_name(pip, ino, child); // which enters (ino, basename) as a dir_entry to the parent INODE;

    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
}

int my_mkdir()
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  my_mdir()\n");
    //(1) divide pathname into dirname and basename
    char *mypath[128];
    char *mydirname[128];
    char *mybasename[128];
    strcpy(mypath, pathname);
    strcpy(mybasename, basename(mypath));
    strcpy(mydirname, get_dirname(mypath));

    printf("mypath = %s\n", mypath);
    printf("mydirname = %s\n", mydirname);
    printf("mybasename = %s\n", mybasename);
    int pino;

    //////////////////////TODO
    // MINODE *wd = running->cwd;
    // ip = &(wd->INODE);
    // if (wd == root)
    // {
    //   printf("wd is root\n");
    //   return;
    // }

    // int my_ino;
    // int parent_ino = findino(wd, my_ino);
    // MINODE *pip = iget(dev, parent_ino);

    //////////////////////

    //(2)

    // printf("mkdir: READ HOW TO pwd in textbook!!!!    %s\n", pino);
    // printf("mkdir: READ HOW TO pwd in textbook!!!!    %s\n", pmip->ino);
    // Verity
    //// check pmip->INODE is a DIR
    if (mydirname[0] == '\0')
    {

        // printf("NOERR: mypath = '/");
        // pino = 2;
        MINODE *wd = running->cwd;
        if (wd == root)
        {
            pino = 2;
        }
        else
        {

            pino = findino2(wd, pino);
        }
    }
    else
    {
        pino = getino(mydirname);
        // if (strcmp(mypath, mybasename))
        // {
        //   printf("ERR: parent not exist\n");
        //   return 0;
        // }
    }

    // parent MINODE.
    MINODE *pmip = iget(dev, pino);

    if (pmip->INODE.i_mode & 0x4000)
    {
        printf("parent is dir\n");
    }
    else
    {
        printf("ERR: parent not dir\n");
        return 0;
    }

    //(3)
    // basename must not exist in parent DIR:
    // search(pmip, basename) must return 0;
    if (search(pmip, mybasename) != 0)
    {
        printf("ERR: basename exist in parent DIR");
        return 0;
    }

    printf("mkdir NO Problem !!!!\n");
    //(4)
    // call kmkdir(pmip, basename) to create a DIR;
    my_kmkdir(pmip, mybasename);

    //(5)
    // increment parent INODE's links_count by 1 and mark pmip dirty;
    iput(pmip);
}

int my_kcreat(MINODE *pip, char child[256])
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  my_kcreat()\n");
    //(1)
    // Allocate an INODE and a disk block
    int ino = ialloc(dev);

    //   (2). no data block is allocated for it, so the
    // file size is 0
    /// int blk = balloc(dev);

    //(2)
    // load INODE into a minode
    MINODE *mip = iget(dev, ino);
    INODE *ip = &mip->INODE;

    // initialize mip->INODE as a FILE INODE;
    // (1).the INODE.i_mode field is set to REG fil type,
    //   permission bits set to 0644 = rw - r-- r--, and
    // #define DIR_MODE 0x41ED
    // #define FILE_MODE 0x81AE
    ip->i_mode = 0x81AE;

    ip->i_uid = running->uid; // owner uid
    ip->i_gid = running->gid; // group Id
    ip->i_size = 0;           // size in bytes
    //   (3). links_count = 1; Do not increment parent
    // INODE’s links_count
    ip->i_links_count = 1; // links count=2 because of . and ..
    ip->i_atime = ip->i_ctime = ip->i_mtime = time(0L);
    ip->i_blocks = 0; // LINUX: Blocks count in 512-byte chunks
    // mip->INODE.i_block[0] = blk;
    // ip->i_block[0] = blk; // new DIR has one datablock
    //                       // other i_block[ ] = 0;

    for (int i = 0; i < 15; i++)
    {
        mip->INODE.i_block[i] = 0;
    }
    // mark minode modified (dirty);
    mip->dirty = 1;
    // write INODE back to disk

    iput(mip);

    // //(3)
    // // make data block 0 of INODE to contain . and .. entries;
    // // write to disk block blk.
    // char buf[BLKSIZE];
    // bzero(buf, BLKSIZE); // optional: clear buf[ ] to 0 DIR *dp = (DIR *)buf;
    // // make . entry
    // DIR *dp = (DIR *)buf;

    // dp->inode = ino;

    // dp->rec_len = 12;

    // dp->name_len = 1;

    // dp->name[0] = '.';

    // // make .. entry: pino=parent DIR ino,
    // // blk = allocated block!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // dp = (char *)dp + 12;
    // dp->inode = pip->ino;
    // dp->rec_len = BLKSIZE - 12; // rec_len spans block
    // dp->name_len = 2;
    // dp->name[0] = dp->name[1] = '.';
    // put_block(dev, blk, buf); // write to blk on  disk

    //(4)
    enter_name(pip, ino, child); // which enters (ino, basename) as a dir_entry to the parent INODE;
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
}

int my_creat()
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  my_creat()\n");
    //(1) divide pathname into dirname and basename
    char *mypath[128];
    char *mydirname[128];
    char *mybasename[128];
    strcpy(mypath, pathname);
    strcpy(mybasename, basename(mypath));
    strcpy(mydirname, get_dirname(mypath));

    printf("mypath = %s\n", mypath);
    printf("mydirname = %s\n", mydirname);
    printf("mybasename = %s\n", mybasename);
    int pino;
    ///////TODO
    if (mydirname[0] == '\0')
    {

        // printf("NOERR: mypath = '/");
        // pino = 2;
        MINODE *wd = running->cwd;
        if (wd == root)
        {
            pino = 2;
        }
        else
        {

            pino = findino2(wd, pino);
        }
    }
    else
    {
        pino = getino(mydirname);
        // Verity
        //// check pmip->INODE is a DIR
        // if (strcmp(mypath, mybasename))
        // {
        //   printf("ERR: parent not exist\n");
        //   return 0;
        // }
    }

    MINODE *pmip = iget(dev, pino);

    if (pmip->INODE.i_mode & 0x4000)
    {
        printf("parent is dir\n");
    }
    else
    {
        printf("ERR: parent not dir\n");
        return 0;
    }

    //(3)
    // basename must not exist in parent DIR:
    // search(pmip, basename) must return 0;
    if (search(pmip, mybasename) != 0)
    {
        printf("ERR: basename exist in parent DIR");
        return 0;
    }

    printf("creat NO Problem !!!!\n");
    //(4)
    // call kmkdir(pmip, basename) to create a DIR;
    my_kcreat(pmip, mybasename);

    //(5)
    // increment parent INODE's links_count by 1 and mark pmip dirty;
    // iput(pmip);
    // (3).links_count = 1;
    // Do not increment parent
    //     INODE’s links_count
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  \n");
}