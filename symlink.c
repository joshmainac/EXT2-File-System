#include "functions.h"
// hard link ->point to inode
// point to the actual file
// creat new file with own inode, name has arrow

int my_ksymlink(MINODE *pip, char child[256])
{
    //(1)
    // Allocate an INODE and a disk block
    int ino = ialloc(dev);

    //   (2). no data block is allocated for it, so the
    // file size is 0
    int blk = balloc(dev);

    //(2)
    // load INODE into a minode
    MINODE *mip = iget(dev, ino);
    INODE *ip = &mip->INODE;

    // initialize mip->INODE as a FILE INODE;
    // (1).the INODE.i_mode field is set to REG fil type,
    //   permission bits set to 0644 = rw - r-- r--, and
    // #define DIR_MODE 0x41ED
    // #define FILE_MODE 0x81AE
    // #define SYM_LNK_MODE 0xA1ED
    ip->i_mode = 0x81AE;
    ip->i_mode = 0120000; // symlink

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
}

int my_symlink()
{

    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  my_symlink()\n");
    char cmd[256], old_file[256], new_file[256]; // get this from line
    sscanf(line, "%s %s %s", cmd, old_file, new_file);
    printf("cmd=%s old_file=%s new_file=%s\n", cmd, old_file, new_file);

    // get ino & mino of old_file
    int oino = getino(old_file);
    printf("oino %d\n", oino);
    MINODE *omip = iget(dev, oino);

    // new_file. get pino & pmip
    //(parent DIR we want to make child under)
    //  separate new_file into parent(dir) and child(basename)
    char mypath[256], parent[256], child[256];
    strcpy(parent, get_dirname(new_file));
    strcpy(child, basename(new_file));
    printf("new_file parent %s\n", parent);
    printf("new_file child %s\n", child);
    int pino = getino(parent);
    if (strcmp(parent, "") == 0)
    {
        printf("No parent for new_file.\n");
        printf("Make new_file at wd\n");
        printf("wd is the parent\n");
        MINODE *wd = running->cwd;
        pino = search(wd, ".");
    }
    printf("pino %d\n", pino);
    if (pino == 0)
    {
        pino = 2;
        printf("pino = 0 --> pino = %d\n", pino);
    }
    MINODE *pmip = iget(dev, pino);

    ////!!!!!

    // pino = 2;
    // pmip = iget(dev, pino);
    // strcpy(child, basename(new_file));
    // make child under pmip
    my_kcreat(pmip, child);
    iput(pmip);

    // get child ino
    int ino = getino(new_file);
    MINODE *mip = iget(dev, ino);
    INODE *ip = &mip->INODE;
    ip->i_mode = 0120000; // symlink
    ip->i_size = strlen(old_file);
    memcpy(ip->i_block, old_file, strlen(old_file));
    mip->dirty = 1;
    // iput(mip);

    //(5)
    // increment parent INODE's links_count by 1 and mark pmip dirty;
    // iput(pmip);
    // (3).links_count = 1;
    // Do not increment parent
    //     INODE’s links_count
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  \n");
}

int my_readlink()
{
    char pathname[256];
    char buf[BLKSIZE];
    int ino;
    MINODE *mip;
    // split pathname
    sscanf(line, "%s %s", cmd, pathname);
    printf("%s %s\n", cmd, pathname);

    // (1). verify pathname exists and is a symlink;
    ino = getino(pathname);
    if (ino == 0)
    {
        printf("ERR: pathname not exist\n");
        return 0;
    }
    mip = iget(dev, ino);
    if (mip->INODE.i_mode != 0120000)
    {
        printf("ERR: pathname is not a symlink\n");
        return 0;
    }
    printf("***************readlink***************\n");
    // (2). read data block 0 of INODE to buf[ ];
    // get_block(dev, mip->INODE.i_block[0], buf);
    // printf("readlink-2 %s\n", mip->INODE.i_block[0]);
    // (3). print buf[ ];
    printf("%s\n", (char *)(mip->INODE.i_block));
    printf("******************************\n");
}

int my_utime()
{
    printf("my_utime-1\n");

    int ino, dev;
    MINODE *mip;
    char *name;
    char buf[BLKSIZE];
    DIR *dp;

    if (pathname[0] == '/')
    {
        dev = root->dev;
        mip = root;
        printf("my_utime-2\n");
    }
    else
    {
        dev = running->cwd->dev;
        mip = running->cwd;
        printf("my_utime-3\n");
    }
    ino = getino(pathname);
    if (ino == 0)
    {
        printf("File does not exist\n");
        return 0;
    }
    printf("my_utime-4\n");
    mip = iget(dev, ino);
    printf("my_utime-5  %d\n", mip->INODE.i_atime);
    mip->INODE.i_atime = time(0L);
    printf("my_utime-5  %d\n", mip->INODE.i_atime);
    mip->dirty = 1;
    iput(mip);
    printf("my_utime-5\n");
    return 1;
}
