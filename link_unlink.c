#include "functions.h"

// my unlink
int my_link()
{

    char old_file[256], new_file[256];
    char parent[256], child[256];
    int oino;
    int pino;
    MINODE *omip, *pmip;
    // split pathname old_file and new_file
    char *temp;
    char mypath[256];
    strcpy(mypath, line);
    sscanf(line, "%s %s %s", cmd, old_file, new_file);

    // (1). verify old_file exists and is not a DIR;
    oino = getino(old_file);

    if (oino == 0)
    {
        printf("ERR: old_file not exist\n");
        return 0;
    }

    omip = iget(dev, oino);

    // check omip->INODE file type (must not be DIR).
    if (omip->INODE.i_mode & 0x4000)
    {
        printf("ERR: old_file %s is dir\n", old_file);
        return;
    }
    printf("p5\n");

    // 3). creat new_file with the same inode number of old_file:
    printf("new_file %s\n", new_file);
    printf("old_file %s\n", old_file);
    strcpy(parent, get_dirname(new_file));
    strcpy(child, basename(new_file));
    printf("p6\n");
    pino = getino(parent);
    printf("parent %s\n", parent);
    printf("child %s\n", child);

    if (pino == 0)
    {
        printf("new_file %s\n", new_file);

        if (strcmp(new_file, child))
        {
            printf("ERR: parent not exist\n");
            return 0;
        }
        else
        {

            printf("NOERR: mypath = '/");
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
    }
    // I add this for PA2, if link ERR because of here
    if (strcmp(parent, "") == 0)
    {
        printf("No parent for new_file.\n");
        printf("Make new_file at wd\n");
        printf("wd is the parent\n");
        MINODE *wd = running->cwd;
        pino = search(wd, ".");
    }

    //

    pmip = iget(dev, pino);

    // enter name to parent, same inode but different name

    // oino = ialloc(dev);

    enter_name(pmip, oino, child);

    omip->INODE.i_links_count++;
    omip->dirty = 1; // for write back by iput(omip)
    iput(omip);
    iput(pmip);
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
    return;
}

int my_unlink()
{

    char *mypath[128];
    char *mydirname[128];
    char *mybasename[128];
    char *name[128];
    char parent[256], child[256];
    MINODE *pmip;
    int pino;

    strcpy(mypath, pathname);
    strcpy(mybasename, basename(mypath));
    strcpy(mydirname, get_dirname(mypath));

    printf("mypath = %s\n", mypath);
    printf("mydirname = %s\n", mydirname);
    printf("mybasename = %s\n", mybasename);

    // child ino
    int ino = getino(mypath);
    printf("ino=%d\n", ino);

    // child minode
    // iget only recuive inode DIR inode
    // find minode of child

    MINODE *mip = iget(dev, ino);

    // parent ino
    // problem with find ino.
    //////!

    strcpy(parent, mydirname);
    strcpy(child, mybasename);

    //  parent minode

    // child name

    // remove name from parent directory
    //###########
    // MINODE *wd = running->cwd;
    // pino = getino(parent);

    // if (wd == root && pino == 0)
    // {
    //   printf("root pino = %d\n", pino);
    //   pino = 2;
    //   pmip = iget(dev, pino);
    // }
    // else
    // {
    //   printf("NOT root\n");
    //   pino = getino(parent);
    //   pmip = iget(dev, pino);
    // }
    //###########

    if (parent[0] == '\0')
    {

        // printf("NOERR: mypath = '/");
        // pino = 2;
        MINODE *wd = running->cwd;
        if (wd == root)
        {
            printf("root\n");
            pino = 2;
        }
        else
        {
            printf("NOT root-1\n");
            pino = findino2(wd, pino);
            printf("NOT root-1  pino = %d\n", pino);
        }
    }
    else
    {
        pino = getino(parent);
        printf("parent = %s\n", parent);
        printf("NOT root-2  pino = %d\n", pino);
        // Verity
        //// check pmip->INODE is a DIR
        // if (strcmp(mypath, mybasename))
        // {
        //   printf("ERR: parent not exist\n");
        //   return 0;
        // }
    }

    pmip = iget(dev, pino);
    printf("search for [%s] in MINODE pmip = [%d, %d]\n", child, pmip->dev, pmip->ino);
    printf("pino = %d\n", pino);
    rm_child(pmip, child);
    mip->dirty = 1;
    iput(pmip);

    //(3). decrement INODE's link_count by 1
    mip->INODE.i_links_count--;
    //(4). if (mip->INODE.i_links_count > 0)
    if (mip->INODE.i_links_count > 0)
    {
        mip->dirty = 1;
    }
    else
    {
        //(5). deallocate its data blocks and inode
        my_bdalloc(mip->dev, mip->INODE.i_block[0]);
        my_idalloc(mip->dev, mip->ino);
    }
    iput(mip);
}