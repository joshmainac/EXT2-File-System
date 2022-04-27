#include "functions.h"

// typedef struct Mount
// {
//   int dev;     // dev (opened vdisk fd number) 0 means FREE
//   int ninodes; // from superblock
//   int nblocks;
//   int bmap; // from GD block
//   int imap;
//   int iblk;
//   struct Minode *mounted_inode;
//   char name[64];       // device name, e.g. mydisk
//   char mount_name[64]; // mounted DIR pathname
// } MOUNT;

// Ask for filesys (a virtual disk) and mount_point (a DIR pathname)
int mount() /*  Usage: mount filesys mount_point OR mount */
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  mount()\n");
    char cmd[256], my_file[256], mount_point[256]; // get this from line
    sscanf(line, "%s %s %s", cmd, my_file, mount_point);
    printf("cmd=%s my_file=%s mount_point=%s\n", cmd, my_file, mount_point);

    //     1. Ask for filesys (a virtual disk) and mount_point (a DIR pathname).
    //    If no parameters: display current mounted filesystems.

    // 2. Check whether filesys is already mounted:
    //    (you may store mounted filesys name in the MOUNT table entry).
    //    If already mounted, reject;
    //    else: allocate a free MOUNT table entry (dev=0 means FREE).

    int i;
    for (i = 0; i < 8; i++)
    {
        if (mountTable[i].dev != 0 && mountTable[i].name == my_file)
        {
            printf("%s mounted on %s\n", mountTable[i].name, mountTable[i].mounted_inode); // mounted_inode=DIR
            return -1;
        }
    }

    // 3. LINUX open filesys for RW; use its fd number as the new DEV;
    //    Check whether it's an EXT2 file system: if not, reject.
    printf("checking EXT2 FS ....");
    if ((fd = open(my_file, O_RDWR)) < 0)
    {
        printf("open %s failed\n", my_file);
        printf("%s is locked. DO chmod 0777 %s\n", my_file);
        exit(1);
    }

    // dev = fd; // Do not set as global yet.

    // 4. For mount_point: find its ino, then get its minode:
    int ino = getino(mount_point); // get ino:
    printf("ino=%d\n", ino);
    printf("dev=%d\n", dev);
    MINODE *mip = iget(dev, ino); // get minode in memory;

    // 5. Verify mount_point is a DIR.  // can only mount on a DIR, not a file
    //    Check mount_point is NOT busy (e.g. can't be someone's CWD)

    if (mip->INODE.i_mode & 0x4000)
    {
        printf("%s is a DIR\n", mount_point);
    }
    else
    {
        printf("%s is not a DIR\n", mount_point);
        return -1;
    }

    // 6. Allocate a FREE (dev=0) mountTable[] for newdev;
    //  Record new DEV, ninodes, nblocks, bmap, imap, iblk in mountTable[]

    char buf[BLKSIZE];
    /********** read super block  ****************/
    get_block(fd, 1, buf);
    sp = (SUPER *)buf;

    /* verify it's an ext2 file system ***********/
    if (sp->s_magic != 0xEF53)
    {
        printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
        exit(1);
    }
    printf("EXT2 FS OK\n");
    int my_ninodes = sp->s_inodes_count;
    int my_nblocks = sp->s_blocks_count;

    get_block(fd, 2, buf);
    gp = (GD *)buf;

    int my_bmap = gp->bg_block_bitmap;
    int my_imap = gp->bg_inode_bitmap;
    int my_iblk = gp->bg_inode_table;
    printf("bmp=%d imap=%d inode_start = %d\n", my_bmap, my_imap, my_iblk);

    // allocate a free MOUNT table entry (dev=0 means FREE)
    for (i = 0; i < 8; i++)
    {
        if (mountTable[i].dev == 0)
        {
            mountTable[i].dev = fd; // opened in 3.
            mountTable[i].ninodes = my_ninodes;
            mountTable[i].nblocks = my_nblocks;
            mountTable[i].bmap = my_bmap;
            mountTable[i].imap = my_imap;
            mountTable[i].iblk = my_iblk;
            mountTable[i].mounted_inode = mip;             // This points ar DIR. declared in 4.
            strcpy(mountTable[i].name, my_file);           // device name
            strcpy(mountTable[i].mount_name, mount_point); // mounted DIR pathname

            break;
        }
    }

    // 7. Mark mount_point's minode as being mounted on and let it point at the
    //    MOUNT table entry, which points back to the mount_point minode.
    mip->mounted = 1;

    // kese
    for (i = 0; i < 8; i++)
    {
        if (mountTable[i].dev != 0)
        {
            MINODE *mymip = mountTable[i].mounted_inode;
            printf("mountTable[%d]\n", i);
            printf("%s mounted on %d\n", mountTable[i].name, mymip->ino);
            printf("dev=%d ninodes=%d nblocks=%d bmap=%d imap=%d iblk=%d\n", mountTable[i].dev, mountTable[i].ninodes, mountTable[i].nblocks, mountTable[i].bmap, mountTable[i].imap, mountTable[i].iblk);
            printf("mounted_inode=%d\n", mymip->ino);
            printf("name=%s\n", mountTable[i].name);
            printf("mount_name=%s\n", mountTable[i].mount_name);
            // savemymip-> the original DOR
            // iput(mymip); // Not sure what to do with this
        }
        else
        {
            printf("mountTable[%d] is FREE\n", i);
        }
    }

    // ERR,not sure if this is right.
    iput(mip);

    //

    return 0;
}

// Ask for filesys (a virtual disk)
// int umount(char *filesys)
int umount()
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  umount()\n");
    char cmd[256], my_file[256], mount_point[256]; // get this from line
    sscanf(line, "%s %s %s", cmd, my_file);
    printf("cmd=%s my_file=%s \n", cmd, my_file);

    // 1. Search the MOUNT table to check filesys is indeed mounted.

    int i;
    for (i = 0; i < 8; i++)
    {
        printf("mountTable[%d]   ", i);
        printf("%s mounted on %d\n", mountTable[i].name, mountTable[i].dev);
        printf("mountTable[].dev=%d  mountTable[].name=%s\n  ", mountTable[i].dev, mountTable[i].name);
        if (mountTable[i].dev != 0 && strcmp(mountTable[i].name, my_file) == 0)
        {
            printf("!!!!F0und %s mounted on %s\n", mountTable[i].name, mountTable[i].mounted_inode); // mounted_inode=DIR
            break;
        }
    }

    if (i == 8)
    {
        printf("%s is not mounted\n", my_file);
        return -1;
    }

    //  2. Check whether any file is still active in the mounted filesys;
    //    e.g. someone's CWD or opened files are still there,
    // if so, the mounted filesys is BUSY ==> cannot be umounted yet.
    // HOW to check?      ANS: by checking all minode[].dev with refCount>0
    if (dev != 3)
    {
        printf("%s is busy\n", my_file);
        return -1;
    }

    // 3. Find the mount_point's inode (which should be in memory while it's mounted on).
    // Reset it to "not mounted"; then
    // iput() the minode.  (because it was iget()ed during mounting)

    MINODE *mymip = mountTable[i].mounted_inode;
    mymip->mounted = 0;
    iput(mymip);

    // 4. return 0 for SUCCESS;
    return 0;
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
int my_access(char *filename, char target_mode)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  access()\n");
    char cmd[256], my_file[256], my_mode[256]; // ge.t this from line
    sscanf(line, "%s %s %s", cmd, my_file, my_mode);
    printf("cmd=%s my_file=%s my_mode=%s \n", cmd, my_file, my_mode);
    strcpy(my_file, filename);
    strcpy(my_mode, &target_mode);

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

// ps (show process queue as Pi[uid]==>}
// m checks the process uid if match permission
int my_ps()
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  ps()\n");

    int i;
    for (i = 0; i < NPROC; i++)
    {
        printf("p%d[uid=%d&pid=%d]=>", i, proc[i].uid, proc[i].pid);
    }
    printf("\n");
    printf("current process = p%d\n", running->pid);

    return 0;
}

// usually all inode->uid is 0 which is the root uid
int my_cs()
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  cs()\n");
    printf("running->pid=%d\n", running->pid);

    if (running->pid == 0)
    {
        printf("switching from p0 to p1\n");
        running = &proc[1];
        running->uid = 1;
        running->cwd = iget(dev, 2);
    }
    else
    {
        printf("switching from p1 to p0\n");
        running = &proc[0];
    }
    printf("running->pid=%d\n", running->pid);

    return 0;
}
