/*********** util.c file ****************/

#include "functions.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <fcntl.h>
// #include <ext2fs/ext2_fs.h>
// #include <string.h>
// #include <libgen.h>
// #include <sys/stat.h>
// #include <time.h>

// #include "type.h"

/**** globals defined in main.c file ****/
// extern MINODE minode[NMINODE];
// extern MINODE *root;
// extern PROC   proc[NPROC], *running;

// extern char gpath[128];
// extern char *name[64];
// extern int n;

// extern int fd, dev;
// extern int nblocks, ninodes, bmap, imap, iblk;

// extern char line[128], cmd[32], pathname[128];

int get_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk * BLKSIZE, 0);
   read(dev, buf, BLKSIZE);
}

int put_block(int dev, int blk, char *buf)
{
   lseek(dev, (long)blk * BLKSIZE, 0);
   write(dev, buf, BLKSIZE);
}

int tokenize(char *pathname)
{
   int i;
   char *s;
   printf("tokenize %s\n", pathname);

   strcpy(gpath, pathname); // tokens are in global gpath[ ]
   n = 0;

   s = strtok(gpath, "/");
   while (s)
   {
      name[n] = s;
      n++;
      s = strtok(0, "/");
   }
   name[n] = 0;

   for (i = 0; i < n; i++)
      printf("%s  ", name[i]);
   printf("\n");
}

// return minode pointer to loaded INODE
// summary
// pass in ino
//
//
//
//
//

MINODE *iget(int dev, int ino)
{
   int i;
   MINODE *mip;
   char buf[BLKSIZE];
   int blk, offset;
   INODE *ip;

   // go through minode array
   // if ref count not 0 & dev & ino match
   // increase ref count and return that inode
   for (i = 0; i < NMINODE; i++)
   {
      mip = &minode[i];
      if (mip->refCount && mip->dev == dev && mip->ino == ino)
      {
         mip->refCount++;
         // printf("found [%d %d] as minode[%d] in core\n", dev, ino, i);
         return mip;
      }
   }

   // make new minode
   for (i = 0; i < NMINODE; i++)
   {
      mip = &minode[i];
      if (mip->refCount == 0)
      {
         // printf("allocating NEW minode[%d] for [%d %d]\n", i, dev, ino);
         mip->refCount = 1;
         mip->dev = dev;
         mip->ino = ino;
         // mip->mounted = 0;

         // get INODE of ino to buf
         blk = (ino - 1) / 8 + iblk;
         offset = (ino - 1) % 8;

         // printf("iget: ino=%d blk=%d offset=%d\n", ino, blk, offset);

         get_block(dev, blk, buf);
         ip = (INODE *)buf + offset;
         // copy INODE to mp->INODE
         mip->INODE = *ip;
         return mip;
      }
   }
   printf("PANIC: no more free minodes\n");
   return 0;
}

void iput(MINODE *mip)
{
   int i, block, offset;
   char buf[BLKSIZE];
   INODE *ip;

   if (mip == 0)
      return;

   mip->refCount--;

   if (mip->refCount > 0)
      return;
   if (!mip->dirty)
      return;

   /* write INODE back to disk */
   /**************** NOTE ******************************
    For mountroot, we never MODIFY any loaded INODE
                   so no need to write it back
    FOR LATER WROK: MUST write INODE back to disk if refCount==0 && DIRTY

    Write YOUR code here to write INODE back to disk

   *****************************************************/
   block = ((mip->ino - 1) / 8) + iblk;
   offset = (mip->ino - 1) % 8;

   // get block containing this inode
   get_block(mip->dev, block, buf);
   ip = (INODE *)buf + offset;      // ip points at INODE
   *ip = mip->INODE;                // copy INODE to inode in block
   put_block(mip->dev, block, buf); // write back to disk
   // midalloc(mip);
}

int search(MINODE *mip, char *name)
{
   printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  search()\n");
   int i;
   char *cp, c, sbuf[BLKSIZE], temp[256];
   DIR *dp;
   INODE *ip;

   printf("search for %s in MINODE = [%d, %d]\n", name, mip->dev, mip->ino);
   ip = &(mip->INODE);

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   get_block(dev, ip->i_block[0], sbuf);
   dp = (DIR *)sbuf;
   cp = sbuf;
   printf("  ino   rlen  nlen  name\n");

   while (cp < sbuf + BLKSIZE)
   {
      strncpy(temp, dp->name, dp->name_len);
      temp[dp->name_len] = 0;
      printf("%4d  %4d  %4d    %s\n",
             dp->inode, dp->rec_len, dp->name_len, dp->name);
      if (strcmp(temp, name) == 0)
      {
         printf("found %s : ino = %d\n", temp, dp->inode);
         return dp->inode;
      }
      cp += dp->rec_len;
      dp = (DIR *)cp;
   }

   printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  \n");
   return 0;
}

int getino(char *pathname)
{
   int i, ino, blk, offset;
   char buf[BLKSIZE];
   INODE *ip;
   MINODE *mip;
   printf("@@@@@@@@@@@@@@@@@@@@@@@@@@  getino(%s)\n", pathname);
   printf("getino: pathname=%s\n", pathname);
   if (strcmp(pathname, "/") == 0)
   {
      printf("getino: root\n");
      printf("@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
      return 2;
   }
   // This is my code
   // if (strcmp(pathname, "") == 0)
   // {
   //    printf("getino: No parent. return 2\n");
   //    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
   //    return 2;
   // }
   // This is my code

   // starting mip = root OR CWD
   if (pathname[0] == '/')
      mip = root;
   else
      mip = running->cwd;

   mip->refCount++; // because we iput(mip) later

   tokenize(pathname);

   for (i = 0; i < n; i++)
   {
      printf("i=%d n=%d\n", i, n);
      printf("===========================================\n");
      printf("getino: i=%d name[%d]=%s\n", i, i, name[i]);

      ino = search(mip, name[i]);

      if (ino == 0)
      {
         iput(mip);
         printf("name %s does not exist\n", name[i]);
         printf("@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
         return 0;
      }
      iput(mip);
      printf("From [%d,%d]\n", mip->dev, mip->ino);
      printf("To [%d,%d]\n", dev, ino);
      // Upward traversal for mounted DIR
      if (mip->dev == dev && mip->ino == ino && mip->dev != 3 && mip->ino == 2)
      {
         printf("getino() Enter upward traversal\n");

         for (int j = 0; j < 8; j++)
         {
            if (mountTable[j].dev == dev)
            {
               MINODE *mymip = mountTable[i].mounted_inode; //// This points at original DIR mip
               dev = mymip->dev;                            // mounted DIR's dev
               ino = mymip->ino;                            // mounted DIR's ino
               printf("my new dev = %d and ino = %d\n", dev, ino);
            }
         }
         mip = iget(dev, ino);
         ino = search(mip, "..");
         iput(mip);
         mip = iget(dev, ino);
         continue;
      }

      mip = iget(dev, ino);
      // Downward traversal for mounted DIR
      if (mip->mounted) // somehow always mounted when a/mnt
      {
         printf("getino() Enter downwar traversal\n");
         printf("getino: %s is mounted\n", name[i]);
         for (int j = 0; j < 8; j++)
         {
            if (mountTable[j].dev != 0 && mountTable[j].mounted_inode == mip)
            {
               // iput(mip);
               dev = mountTable[j].dev; // 4
               ino = 2;
               mip = iget(dev, 2); // get root MINODE of mounted device
            }
         }
         printf("i=%d n=%d\n", i, n);
      }

      //
   }

   iput(mip);
   printf("@@@@@@@@@@@@@@@@@@@@@@@@@@ \n");
   return ino;
}

// These 2 functions are needed for pwd()
int findmyname(MINODE *parent, u32 myino, char myname[])
{
   // WRITE YOUR code here
   // search parent's data block for myino; SAME as search() but by myino
   // copy its name STRING to myname[ ]
   int i;
   char *cp;
   DIR *dp;
   char sbuf[BLKSIZE], temp[256];

   INODE *ip = &parent->INODE;

   /*** search for name in mip's data blocks: ASSUME i_block[0] ONLY ***/

   for (i = 0; i < 12; i++)
   {
      if (ip->i_block[i] == 0)
      {
         break;
      }
      get_block(dev, ip->i_block[i], sbuf);
      dp = (DIR *)sbuf;
      cp = sbuf;
      while (cp < sbuf + BLKSIZE)
      {

         if (myino == dp->inode)
         {

            strncpy(myname, dp->name, dp->name_len);
            myname[dp->name_len] = 0;
            return 1;
         }
         cp += dp->rec_len;
         dp = (DIR *)cp;
      }
   }

   return 0;
}

int findino(MINODE *mip, u32 *myino) // myino = i# of . return i# of ..
{
   // mip points at a DIR minode
   // WRITE your code here: myino = ino of .  return ino of ..
   // all in i_block[0] of this DIR INODE.
   u32 my_ino = search(mip, ".");
   myino = my_ino;
   printf("myino = %u\n", myino);
   int ino = search(mip, "..");
   // myino = ino;
   return ino;
}

int findino2(MINODE *mip, u32 *myino) // myino = i# of . return i# of ..
{
   // mip points at a DIR minode
   // WRITE your code here: myino = ino of .  return ino of ..
   // all in i_block[0] of this DIR INODE.
   u32 my_ino = search(mip, "..");
   myino = my_ino;
   printf("myino = %u\n", myino);
   int ino = search(mip, ".");
   // myino = ino;
   return ino;
}
