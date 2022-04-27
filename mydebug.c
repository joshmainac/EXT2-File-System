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
        printf("(%d)minode=[%d,%d]->mounted=%d->refcount=%d->dirty=%d->uid=%d\n", i, mip->dev, mip->ino, mip->mounted, mip->refCount, mip->dirty, mip->INODE.i_uid);
    }

    return 0;
}

int my_dev()
{

    MINODE *wd = running->cwd;
    printf("my MINODE= [%d.%d]\n", dev, wd->ino);

    return 0;
}

int check_dir()
{
    // file  ugs? owner group other
    // 0100 000 000 000 000
    // 1000 REG file, 0100 for DIR, 0010 for FILE
    //(hex)F000 == (int)61440 == (oct)170000 == (bin)1111000000000000
    //(int)32768 == (hex,16,x)8000 ==(oct,8,o)10000 == (bin)1000 000 000 000 000(REG)
    //(int)16384 == (hex,16,x)4000 ==(oct,8,o)40000 == (bin)0100 000 000 000 000(DIR)
    //(int)40960 == (hex,16,x)A000 ==(oct,8,o)120000 == (bin)1010 000 000 000 000(LNK)
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  access()\n");
    char cmd[256], my_file[256]; // ge.t this from line
    sscanf(line, "%s %s", cmd, my_file);
    printf("cmd=%s my_file=%s \n", cmd, my_file);

    int ino = getino(my_file);
    MINODE *mip = iget(dev, ino);
    printf("mode = mip->i_mode\n");
    u16 mode = mip->INODE.i_mode;

    printf("(10)mode=%d\n", mode);
    printf("(8)mode=%o\n", mode);
    printf("(16)mode=%x\n", mode);
    printf("(u)mode=%u\n", mode);
    printf("\n");
    printf("(10)(mode & 0xF000)=%d\n", (mode & 0xF000));
    printf("(8)(mode & 0xF000)=%o\n", (mode & 0xF000));
    printf("(16)(mode & 0xF000)=%x\n", (mode & 0xF000));
    printf("(u)(mode & 0xF000)=%u\n", (mode & 0xF000));
    printf("\n");

    //(10)mode = (10)16877-->(2)1000 001 111 011 01
    //(16)F000= (2)1111 000 000 000 000
    //(16)mode&F000= (10)16384 -->(2)100 000 000 000 000-->(2)0100 000 000 000 000
    // DIR = (2)0100 000 000 000 000
}

// file  ugs? owner group other
// 0100 000 000 000 000
// ----------------- Define these bit patterns -----------------------
// ownerR: b'0000 000 |100 000 000|' = 0000 0001 0000 0000 = 0x0100
// ownerW: b'0000 000 |010 000 000|' = 0000 0000 1000 0000 = 0x0080
// ownerX: b'0000 000 |001 000 000|' = 0000 0000 0100 0000 = 0x0040
// -------------------------------------------------------------------

// 0000 000 111 000 000(2) == 1C0(16)

// ----------------- Define these bit patterns -----------------------
// otherR: b'0000 000 |000 000 100|'= 0000 0000 0000 0100 = 0x0004
// otherW: b'0000 000 |000 000 010|' = 0000 0000 0000 0010 = 0x0002
// otherX: b'0000 000 |000 000 001|' = 0000 0000 0000 0001 = 0x0001
// -------------------------------------------------------------------

// 0000 000 000 000 111(2) == 7(16)

//'0000 000 100 000 000 = |0000 |000 1 |00 00 |0 000
// return 1 if can access, 0 if not
//(char *filename, char mode)
int my_ac()
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  access()\n");
    char cmd[256], my_file[256], my_mode[256]; // ge.t this from line
    sscanf(line, "%s %s %s", cmd, my_file, my_mode);
    printf("cmd=%s my_file=%s my_mode=%s \n", cmd, my_file, my_mode);

    int r = 0;

    if (running->uid == 0)
    {
        // uid = 0 -->super user, always ok
        printf("uid = 0 = superuser. return 1\n");

        return 1;
    }

    // NOT SUPERuser: get file's INODE
    int ino = getino(my_file);
    MINODE *mip = iget(dev, ino);
    u16 mode = mip->INODE.i_mode;
    ////////////////////////////////////////////////////uid == uid
    if (mip->INODE.i_uid == running->uid)
    {
        // uid = owner [000] 000 000
        //  r = (check owner's rwx with mode);  // by tst_bit()
        printf("uid = owner\n");

        if (strcmp(my_mode, "r") == 0)
        {

            // r = tst_bit(mip->INODE.i_mode, 0);
            // 0000 000 100 000 000(2) == 0x0100(16)  remove except owner-R
            printf("chcking if owner-r is permitted\n");
            if ((mode & 0x0100) == 0x0100)
            {
                r = 1;

                printf("owner-r is OK. return 1\n");
                return r;
            }
            else
            {
                printf("owner-r is NOT OK. return 0\n");
                r = 0;
                return r;
            }
        }

        if (strcmp(my_mode, "w") == 0)
        {
            // r = tst_bit(mip->INODE.i_mode, 0);
            // 0000 000 010 000 000(2) == 0x0100(16)  remove except owner-R
            printf("chcking if owner-w is permitted\n");
            if ((mode & 0x0080) == 0x0080)
            {
                r = 1;

                printf("owner-r is OK. return 1\n");
                return r;
            }
            else
            {
                printf("owner-r is NOT OK. return 0\n");
                r = 0;
                return r;
            }
        }

        if (strcmp(my_mode, "x") == 0)
        {
            // r = tst_bit(mip->INODE.i_mode, 0);
            // 0000 000 001 000 000(2) == 0x0100(16)  remove except owner-R
            printf("chcking if owner-x is permitted\n");
            if ((mode & 0x0040) == 0x0040)
            {
                r = 1;

                printf("owner-r is OK. return 1\n");
                return r;
            }
            else
            {
                printf("owner-r is NOT OK. return 0\n");
                r = 0;
                return r;
            }
        }
    }
    ////////////////////////////////////////////////////

    ////////////////////////////////////////////////////uid != uid
    if (mip->INODE.i_uid != running->uid)
    {
        // uid = owner [000] 000 000
        //  r = (check owner's rwx with mode);  // by tst_bit()
        printf("uid = other\n");

        if (strcmp(my_mode, "r") == 0)
        {

            // r = tst_bit(mip->INODE.i_mode, 0);
            // 0000 000 000 000 100(2) == 0x0004(16)  remove except owner-R
            printf("chcking if other-r is permitted\n");
            if ((mode & 0x0004) == 0x0004)
            {
                r = 1;

                printf("other-r is OK. return 1\n");
                return r;
            }
            else
            {
                printf("other-r is NOT OK. return 0\n");
                r = 0;
                return r;
            }
        }

        if (strcmp(my_mode, "w") == 0)
        {
            // r = tst_bit(mip->INODE.i_mode, 0);
            // 0000 000 000 000 010(2) == 0x0002(16)  remove except owner-R
            printf("chcking if other-w is permitted\n");

            if ((mode & 0x0002) == 0x0002)
            {
                r = 1;

                printf("other-w is OK. return 1\n");
                return r;
            }
            else
            {
                printf("other-w is NOT OK. return 0\n");
                r = 0;
                return r;
            }
        }

        if (strcmp(my_mode, "x") == 0)
        {
            // r = tst_bit(mip->INODE.i_mode, 0);
            // 0000 000 000 000 001(2) == 0x0001(16)  remove except owner-R
            printf("chcking if other-x is permitted\n");
            if ((mode & 0x0001) == 0x0001)
            {
                r = 1;

                printf("other-x is OK. return 1\n");
                return r;
            }
            else
            {
                printf("other-x is NOT OK. return 0\n");
                r = 0;
                return r;
            }
        }
    }
    ////////////////////////////////////////////////////

    iput(mip);

    return r;
}