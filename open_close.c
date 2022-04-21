#include "functions.h"

// open file2 1
// open [file_name] [0,1 or r,w]
int open_file()
{
    char cmd[256], my_file[256], my_mode[256]; // get this from line
    sscanf(line, "%s %s %s", cmd, my_file, my_mode);
    printf("cmd=%s my_file=%s my_mode=%s\n", cmd, my_file, my_mode);
    //(1). get file's minode:
    int ino = getino(my_file);

    // if file does not exist
    if (ino == 0)
    {
        printf("ERR: file does not exist\n");
        return -1;
        // TODO create file(mod this)
        // my_creat();             // creat it first, then
        // ino = getino(pathname); // get its ino
    }

    MINODE *mip = iget(dev, ino);

    // check mip->INODE.i_mode to verify it's a REGULAR file (not dir)
    if (!S_ISREG(mip->INODE.i_mode))
    {
        printf("%s is not a regular file\n", my_file);
        return -1;
    }
    // check permission OK.

    //(2)check if file is already opened. Reject if opened(except for R)
    int i;
    for (i = 0; i < NFD; i++)
    {
        // int ino = getino(pathname);
        // MINODE *mip = iget(dev, ino);
        if (running->fd[i] && running->fd[i]->minodePtr == mip)
        {
            printf("file is already opened\n");
            //  mode = 0|1|2|3 for R|W|RW|APPEND
            if (running->fd[i]->mode != 0)
            {
                printf("file mode = 1|2|3 for W|RW|APPEND so reject\n");
                return -1;
            }
        }
    }
    //(2). allocate an openTable entry OFT; initialize OFT entries:
    // mode = 0(RD) or 1(WR) or 2(RW) or 3(APPEND)
    // MINODE *minodePtr = mip; // point to file’s minode
    // allocate a FREE OpenFileTable (OFT) and fill in values:
    int fd = 0;
    for (i = 0; i < NFD; i++)
    {
        if (running->fd[i] == 0)
        {
            fd = i;
            break;
        }
    }

    OFT *oftp = (OFT *)malloc(sizeof(OFT));

    oftp->refCount = 1;
    // ino = getino(file_name); // get its ino
    // MINODE *mip = iget(dev, ino);
    oftp->minodePtr = mip;
    // set offset = 0 for RD|WR|RW; set to file size for APPEND mode;

    // get mode
    if (strcmp(my_mode, "0") == 0)
    {
        oftp->mode = 0;
        oftp->offset = 0;
    }
    else if (strcmp(my_mode, "1") == 0)
    {
        oftp->mode = 1;
        oftp->offset = 0;
        my_truncate(mip);
    }
    else if (strcmp(my_mode, "2") == 0)
    {
        oftp->mode = 2;
        oftp->offset = 0;
    }
    else if (strcmp(my_mode, "3") == 0)
    {
        oftp->mode = 3;
        oftp->offset = mip->INODE.i_size;
    }
    else
    {
        printf("ERR: invalid mode\n");
        return -1;
    }

    running->fd[fd] = oftp;

    //(3). Search for the first FREE fd[index]
    // entry with the lowest index in PROC;
    // fd[index] = &OFT; // fd entry points to the OFT entry
    //(4). return fd[index],  index as file descriptor
    mip->dirty = 1;

    return fd;
}

// use this in cat//only use in cat
int myopen(char *filename, int flags)
{
    char cmd[256], my_file[256], my_mode[256]; // get this from line
    // sscanf(line, "%s %s %s", cmd, my_file, my_mode);
    // printf("cmd=%s my_file=%s my_mode=%s\n", cmd, my_file, my_mode);
    strcpy(my_file, filename);
    //(1). get file's minode:
    int ino = getino(my_file);

    // if file does not exist
    if (ino == 0)
    {
        printf("ERR: file does not exist\n");
        return -1;
        // TODO create file(mod this)
        // my_creat();             // creat it first, then
        // ino = getino(pathname); // get its ino
    }

    MINODE *mip = iget(dev, ino);

    // check mip->INODE.i_mode to verify it's a REGULAR file (not dir)
    if (!S_ISREG(mip->INODE.i_mode))
    {
        printf("%s is not a regular file\n", my_file);
        return -1;
    }
    // check permission OK.

    //(2)check if file is already opened. Reject if opened(except for R)
    int i;
    for (i = 0; i < NFD; i++)
    {
        // int ino = getino(pathname);
        // MINODE *mip = iget(dev, ino);
        if (running->fd[i] && running->fd[i]->minodePtr == mip)
        {
            printf("file is already opened\n");
            //  mode = 0|1|2|3 for R|W|RW|APPEND
            if (running->fd[i]->mode != 0)
            {
                printf("file mode = 1|2|3 for W|RW|APPEND so reject\n");
                return -1;
            }
        }
    }
    //(2). allocate an openTable entry OFT; initialize OFT entries:
    // mode = 0(RD) or 1(WR) or 2(RW) or 3(APPEND)
    // MINODE *minodePtr = mip; // point to file’s minode
    // allocate a FREE OpenFileTable (OFT) and fill in values:
    int fd = 0;
    for (i = 0; i < NFD; i++)
    {
        if (running->fd[i] == 0)
        {
            fd = i;
            break;
        }
    }

    OFT *oftp = (OFT *)malloc(sizeof(OFT));
    oftp->refCount = 1;
    // ino = getino(file_name); // get its ino
    // MINODE *mip = iget(dev, ino);
    oftp->minodePtr = mip;
    // set offset = 0 for RD|WR|RW; set to file size for APPEND mode;

    // get mode
    if (flags == 0)
    {
        oftp->mode = 0;
        oftp->offset = 0;
    }
    else if (flags == 1)
    {
        oftp->mode = 1;
        oftp->offset = 0;
        my_truncate(mip);
    }
    else if (flags == 2)
    {
        oftp->mode = 2;
        oftp->offset = 0;
    }
    else if (flags == 3)
    {
        oftp->mode = 3;
        oftp->offset = mip->INODE.i_size;
    }
    else
    {
        printf("ERR: invalid mode\n");
        return -1;
    }

    running->fd[fd] = oftp;

    //(3). Search for the first FREE fd[index]
    // entry with the lowest index in PROC;
    // fd[index] = &OFT; // fd entry points to the OFT entry
    //(4). return fd[index],  index as file descriptor
    mip->dirty = 1;

    return fd;
}

//   1. release mip->INODE's data blocks;
//      a file may have 12 direct blocks, 256 indirect blocks and 256*256
//      double indirect data blocks. release them all.
//   2. update INODE's time field

//   3. set INODE's size to 0 and mark Minode[ ] dirty
int my_truncate(MINODE *mip)
{
    int buf[256];
    int buf2[256];
    int bnumber, i, j;
    // deallocate for direct
    for (i = 0; i < 12; i++)
    {
        if (mip->INODE.i_block[i] == 0)
            continue;
        my_bdalloc(dev, mip->INODE.i_block[i]);
    }
    // Deallocate Indirect blocks
    if (mip->INODE.i_block[12] != 0)
    {
        get_block(dev, mip->INODE.i_block[12], buf);
        for (i = 0; i < 256; i++)
        {
            if (buf[i] == 0)
                continue;
            my_bdalloc(dev, buf[i]);
        }
    }
    // deallocate double indirect blocks
    if (mip->INODE.i_block[13] != 0)
    {
        get_block(dev, mip->INODE.i_block[13], buf);
        for (i = 0; i < 256; i++)
        {
            if (buf[i] == 0)
                continue;
            get_block(dev, buf[i], buf2);
            for (j = 0; j < 256; j++)
            {
                if (buf2[j] == 0)
                    continue;
                my_bdalloc(dev, buf2[j]);
            }
        }
    }

    mip->INODE.i_atime = mip->INODE.i_mtime = time(0L);
    mip->INODE.i_size = 0;
    mip->dirty = 1;
    return 1;
}

int close_file()
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@close_file()\n");
    char cmd[256], my_file[256]; // get this from line
    sscanf(line, "%s %s %s", cmd, my_file);
    printf("cmd=%s my_file=%s\n", cmd, my_file);
    int fd = atoi(my_file);
    printf("fd=%d\n", fd);
    // 1. verify fd is within range.
    if (fd < 0 || fd >= NFD)
    {
        printf("Invalid file descriptor\n");
        return 0;
    }
    // 2. verify running->fd[fd] is pointing at a OFT entry
    if (running->fd[fd] == 0)
    {
        printf("File descriptor is not open\n");
        return 0;
    }
    // 3. The following code segments should be fairly obvious:
    OFT *oftp = running->fd[fd];
    // This will make it not apeear on pfd()
    running->fd[fd] = 0;
    // mip will disapeear when refCount = 0
    // make fd[]->OFT->refCount to 0

    /// kese
    printf("before cloesd\n");
    printf("fd[]->OFT->refCount=%d\n", oftp->refCount);
    printf("fd[]->OFT->minodePtr->refCount=%d\n", oftp->minodePtr->refCount);
    printf("After cloesd\n");

    ///
    oftp->refCount--;
    printf("fd[]->OFT->refCount=%d\n", oftp->refCount);
    if (oftp->refCount > 0)
    {
        printf("ERR(mkdir_creat): OFT refCount > 0\n");
        return 0;
    }
    // 4. if refCount == 0, then free the OFT entry
    // last user of this OFT entry ==> dispose of the Minode[]
    MINODE *mip = oftp->minodePtr;

    iput(mip);
    // make fd[]->OFT->minodePtr->refCount to 0
    printf("fd[]->OFT->minodePtr->refCount=%d\n", mip->refCount);

    return 0;
}

int myclose(int fd)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@myclose()\n");

    printf("fd=%d\n", fd);
    // 1. verify fd is within range.
    if (fd < 0 || fd >= NFD)
    {
        printf("Invalid file descriptor\n");
        return 0;
    }
    // 2. verify running->fd[fd] is pointing at a OFT entry
    if (running->fd[fd] == 0)
    {
        printf("File descriptor is not open\n");
        return 0;
    }
    // 3. The following code segments should be fairly obvious:
    OFT *oftp = running->fd[fd];
    // This will make it not apeear on pfd()
    running->fd[fd] = 0;
    // mip will disapeear when refCount = 0
    // make fd[]->OFT->refCount to 0

    /// kese
    printf("before cloesd\n");
    printf("fd[]->OFT->refCount=%d\n", oftp->refCount);
    printf("fd[]->OFT->minodePtr->refCount=%d\n", oftp->minodePtr->refCount);
    printf("After cloesd\n");

    ///
    oftp->refCount--;
    printf("fd[]->OFT->refCount=%d\n", oftp->refCount);
    if (oftp->refCount > 0)
    {
        printf("ERR(mkdir_creat): OFT refCount > 0\n");
        return 0;
    }
    // 4. if refCount == 0, then free the OFT entry
    // last user of this OFT entry ==> dispose of the Minode[]
    MINODE *mip = oftp->minodePtr;
    iput(mip);
    // make fd[]->OFT->minodePtr->refCount to 0
    printf("fd[]->OFT->minodePtr->refCount=%d\n", mip->refCount);

    return 0;
}
int my_lseek(int fd, int position)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@my_lseek()\n");
    printf("fd=%d position=%d\n", fd, position);
    // 1. verify fd is within range.
    if (fd < 0 || fd >= NFD)
    {
        printf("Invalid file descriptor\n");
        return 0;
    }
    // 2. verify running->fd[fd] is pointing at a OFT entry
    if (running->fd[fd] == 0)
    {
        printf("File descriptor is not open\n");
        return 0;
    }
    // 3. The following code segments should be fairly obvious:
    OFT *oftp = running->fd[fd];
    // 4. set offset = position
    oftp->offset = position;
    return 0;
}

int lseek_file()
{
    char cmd[256], my_file[256], my_position[256]; // get this from line
    sscanf(line, "%s %s %s", cmd, my_file, my_position);
    printf("cmd=%s my_file=%s my_position=%s\n", cmd, my_file, my_position);
    int fd = atoi(my_file);
    printf("fd=%d\n", fd);
    int n = atoi(my_position);
    printf("position=%d\n", n);
    my_lseek(fd, n);
    // return originalPosition
    return 0;
}

int my_pfd()
{
    int i = 0;
    char mode[256];
    printf(" fd     mode    offset    INODE \n");
    printf("----    ----    ------    -------\n");
    while (i < NFD)
    {
        if (running->fd[i] == NULL)
        {
            i++;
            continue;
        }

        if (running->fd[i]->mode == 0)
        {
            strcpy(mode, "R ");
        }
        else if (running->fd[i]->mode == 1)
        {
            strcpy(mode, "W ");
        }
        else if (running->fd[i]->mode == 2)
        {
            strcpy(mode, "RW");
        }
        else if (running->fd[i]->mode == 3)
        {
            strcpy(mode, "A ");
        }
        // printf(" fd     mode    offset    INODE \n");
        printf("i = %d\n", i);
        printf("   %d      %s      %d", i, mode, running->fd[i]->offset);
        printf("[%d,%d]\n", running->fd[i]->minodePtr->dev, running->fd[i]->minodePtr->ino);
        i++;
    }
    printf("--------------------------------------\n");
}

int my_dup(int fd)
{
    return 0;
}

int my_dup2(int fd, int gd)
{
    return 0;
}