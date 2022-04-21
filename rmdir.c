#include "functions.h"

int rm_child(MINODE *pmip, char *name)
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  rm_child()\n");
    //(1). Search parent INODE's data block(s) for the entry of name
    // Go through entry, find name if (strcmp(temp, name) == 0)
    int i;
    char *cp, c, sbuf[BLKSIZE], temp[256];
    DIR *dp;
    INODE *ip;

    // add this!!
    char *cp2;
    DIR *dp2, *dpPrev;
    char buf2[BLKSIZE];
    // get working dir
    MINODE *wd = running->cwd;
    printf("search for [%s] in MINODE pmip = [%d, %d]\n", name, pmip->dev, pmip->ino);
    if (wd == root)
    {
        printf("root\n");
    }
    else
    {
        printf("NOT root\n");
    }

    ip = &(pmip->INODE);
    printf("p.2   INODE of pmip = %d\n", ip);

    /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

    get_block(dev, ip->i_block[0], sbuf);
    dp = (DIR *)sbuf;
    cp = sbuf;
    // add this!!
    dp2 = (DIR *)sbuf;
    dpPrev = (DIR *)sbuf;
    cp2 = sbuf;

    while (cp < sbuf + BLKSIZE)
    {
        strncpy(temp, dp->name, dp->name_len);
        temp[dp->name_len] = 0;

        if (strcmp(temp, name) == 0)
        {

            // found child-name in parent DIR

            // rm_child if-statesments...
            //(1)if only child ...(cp == buf && dp->rec_len == BLKSIZE)
            //(2)if LAST entry in block
            //(3)if  entry is first but not the only entry or in the middle of a block
            //
            //(1)if only child ...(cp == buf && dp->rec_len == BLKSIZE)
            if (cp == sbuf && dp->rec_len == BLKSIZE)
            {
                // deallocate
                int myino = dp->inode;
                my_bdalloc(dev, myino);
                incFreeBlocks(dev);
                return 0;
            }
            // else delete child (mod this)
            else
            {
                // (2)if (last entry in data block)
                if (cp + dp->rec_len == sbuf + BLKSIZE)
                {
                    // pmip->INODE.i_size -= dp->rec_len;
                    pmip->dirty = 1;
                    // TODO we were able to come here but ynable to remove
                    // check if dp is working

                    dp2->rec_len += dp->rec_len;
                    // kese
                    printf("dp->name %s\n", dp->name);
                    printf("dp2->name %s\n", dp2->name);
                    printf("dp->rec_len %4d\n", dp->rec_len);
                    printf("dp2->rec_len %4d\n", dp2->rec_len);

                    //
                    printf("dp->name %s\n", dp->name);
                    printf("dp2->name %s\n", dp2->name);
                    printf("dp->rec_len %4d\n", dp->rec_len);
                    printf("dp2->rec_len %4d\n", dp2->rec_len);
                    put_block(dev, ip->i_block[0], sbuf);

                    return 0;
                }
                //(3)if  entry is first but not the only entry or in the middle of a block
                else
                {
                    pmip->dirty = 1;
                    int size = &sbuf[BLKSIZE] - cp;
                    dp2->rec_len += dp->rec_len;
                    memcpy(dp, cp, size);

                    put_block(dev, ip->i_block[0], sbuf);

                    return 0;
                }
            }
        }
        cp2 = cp;
        dp2 = dp;
        cp += dp->rec_len;
        dp = (DIR *)cp;
    }
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    return 0;
}

int my_rmdir()
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  my_rmdir2())\n");

    char *mypath[128];
    char *mydirname[128];
    char *mybasename[128];
    char *name[128];

    strcpy(mypath, pathname);
    strcpy(mybasename, basename(mypath));
    strcpy(mydirname, get_dirname(mypath));

    printf("mypath = %s\n", mypath);
    printf("mydirname = %s\n", mydirname);
    printf("mybasename = %s\n", mybasename);

    /************ Algorithm of rmdir *************/
    //(1).get in - memory INODE of pathname :
    // child inode
    int ino = getino(mypath);
    MINODE *mip = iget(dev, ino);
    // (2).verify INODE is a DIR(by INODE.i_mode  field);
    // minode is not BUSY(refCount = 1);
    // verify DIR is empty (traverse data blocks for number of entries = 2);
    //(3). /* get parent's ino and inode */
    // parent inode
    int pino = findino(mip, ino); // get pino from .. entry in INODE.i_block[0]
    MINODE *pmip = iget(mip->dev, pino);
    //(4). /* get name from parent DIR’s data block
    // findname(pmip, ino, name);MINODE *parent, u32 myino, char myname[])
    // find name from parent DIR
    findmyname(pmip, ino, name);

    //(5). remove name from parent directory */
    rm_child(pmip, name);
    //(6). dec parent links_count by 1; mark parent pimp dirty;
    iput(pmip);
    //(7). /* deallocate its data blocks and inode */
    my_bdalloc(mip->dev, mip->INODE.i_block[0]);
    my_idalloc(mip->dev, mip->ino);
    iput(mip);
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
}