#include "functions.h"

/************* cd_ls_pwd.c file **************/
int cd()
{
  printf("cd: under construction READ textbook!!!!\n");

  // READ Chapter 11.7.3 HOW TO chdir
  //(1). int ino = getino(pathname); // return error if ino=0
  int ino = getino(pathname);
  //!!!!!

  if (ino == -1 || ino == 0)
  {
    ino = 2;
  }

  //
  //(2). MINODE *mip = iget(dev, ino);
  MINODE *mip = iget(dev, ino);
  //(3). Verify mip->INODE is a DIR // return error
  // if not DIR
  if (!S_ISDIR(mip->INODE.i_mode))
  {
    printf("Not a directory\n");
    iput(mip);
    return 0;
  }
  //(4). iput(running->cwd); // release old cwd
  iput(running->cwd);
  //(5). running->cwd = mip; // change cwd to mip
  running->cwd = mip;
}

int ls_file(MINODE *mip, char *name)
{
  // printf("ls_file: to be done: READ textbook!!!!\n");
  INODE *pip = &mip->INODE;
  u16 mode = pip->i_mode;
  time_t val = pip->i_ctime;
  char *mtime = ctime(&val);
  mtime[strlen(mtime) - 1] = '\0';
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";
  int i;

  if ((mode & 0xF000) == 0x8000) // if (S_ISREG())
    printf("%c", '-');
  if ((mode & 0xF000) == 0x4000) // if (S_ISDIR())
    printf("%c", 'd');
  if ((mode & 0xF000) == 0xA000) // if (S_ISLNK())
    printf("%c", 'l');

  for (i = 8; i >= 0; i--)
  {
    if (mode & (1 << i)) // print r|w|x
      printf("%c", t1[i]);
    else
      printf("%c", t2[i]); // or print -
  }

  printf(" ");
  printf("%4d %4d %4d  %s   %8d   %s", pip->i_links_count, pip->i_gid, pip->i_uid, mtime, pip->i_size, name);
  if ((mode & 0120000) == 0120000)
    printf(" => %s", (char *)(mip->INODE.i_block));
  printf(" [%d, %d]\n", mip->dev, mip->ino);
  printf(" mode %x\n", mode);
  printf(" mode %o\n", pip->i_mode);
}

int ls_dir(MINODE *mip)
{
  // printf("ls_dir: list CWD's file names; YOU FINISH IT as ls -l\n");

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;

  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  int ino;
  MINODE *pip;

  while (cp < buf + BLKSIZE)
  {
    strncpy(temp, dp->name, dp->name_len);
    temp[dp->name_len] = 0;
    ino = dp->inode;
    pip = iget(dev, ino);

    ls_file(pip, temp);
    cp += dp->rec_len;
    dp = (DIR *)cp;
    iput(pip);
  }
  printf("\n");
}

int ls()
{

  unsigned long ino;
  MINODE *mip;
  // MINODE *mip, *pip;
  // char *child;
  mip = running->cwd;

  if (pathname[0] == 0) // print cwd
  {

    ls_dir(mip);
  }
  else
  {

    ino = getino(pathname);

    if (ino == -1 || ino == 0)
    {

      return 1;
    }
    mip = iget(dev, ino);

    ls_dir(mip);
  }

  // iput(mip->dev, mip);
  return 0;
}

int rpwd(MINODE *wd)
{
  // (1). if (wd==root) return;
  INODE *ip;
  ip = &(wd->INODE);
  if (wd == root)
  {
    printf("CWD = ");
    return;
  }
  // printf("(1)\n");
  //  (2). from wd->INODE.i_block[0], get my_ino and parent_ino
  // int my_ino = wd->INODE.i_block[0];

  int my_ino;
  int parent_ino = findino(wd, my_ino);

  // (3). pip = iget(dev, parent_ino);
  MINODE *pip = iget(dev, parent_ino);
  // printf("(3)\n");
  //  (4). from pip->INODE.i_block[ ]: get my_name string by my_ino as LOCAL
  // char *my_name = pip->INODE.i_block[0];
  char myname[256];
  // printf("(4)-a\n");

  findmyname(pip, wd->ino, myname);
  // printf("(4)-b\n");
  //  (5). rpwd(pip); // recursive call rpwd(pip) with parent minode
  rpwd(pip);
  // printf("(5)\n");
  //  (6). print "/%s", my_name;

  printf("/%s ", myname);
  // printf("(6)\n");
}

char *pwd(MINODE *wd)
{
  // printf("creating P0 as running process\n");
  // running = &proc[0];
  // running->cwd = iget(dev, 2);
  // pwd(running->cwd);
  // wd is the running process cwd

  if (wd == root)
  {
    printf("CWD = ");
    printf("/\n");
    return;
  }

  else
  {
    rpwd(wd);
  }
  printf("\n");
}

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

char *get_dirname(char *path)
{
  char *dirname = malloc(strlen(path) + 1);
  strcpy(dirname, path);
  char *p = strrchr(dirname, '/');
  if (p)
    *p = '\0';
  else
    dirname[0] = '\0';
  return dirname;
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

  // get child ino
  int ino = getino(new_file);
  MINODE *mip = iget(dev, ino);
  INODE *ip = &mip->INODE;
  ip->i_mode = 0120000; // symlink
  ip->i_size = strlen(old_file);
  memcpy(ip->i_block, old_file, strlen(old_file));
  mip->dirty = 1;
  iput(mip);

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

// gcc -m32 main.c util.c
// mkdir dir1/j

// 1. stat filename:
//         struct stat myst;
//         get INODE of filename into memory:
//             int ino = getino(pathname);
//             MINODE *mip = iget(dev, ino);
//             copy dev, ino to myst.st_dev, myst.st_ino;
//             copy mip->INODE fields to myst fields;
//         iput(mip);
int my_stat()
{
  char *name;
  char buf[BLKSIZE];
  DIR *dp;
  int ino, dev;
  MINODE *mip;
  struct stat myst;

  if (pathname[0] == '/')
  {
    dev = root->dev;
    mip = root;
    printf("stat_file-2\n");
  }
  else
  {
    dev = running->cwd->dev;
    mip = running->cwd;
    printf("stat_file-3\n");
  }
  ino = getino(pathname);
  if (ino == 0)
  {
    printf("File does not exist\n");
    printf("stat_file-4\n");
    return 0;
  }

  mip = iget(dev, ino);
  myst.st_dev = mip->dev;
  myst.st_ino = mip->ino;
  myst.st_mode = mip->INODE.i_mode;
  myst.st_nlink = mip->INODE.i_links_count;
  myst.st_uid = mip->INODE.i_uid;
  myst.st_gid = mip->INODE.i_gid;
  myst.st_size = mip->INODE.i_size;
  myst.st_atime = mip->INODE.i_atime;
  myst.st_mtime = mip->INODE.i_mtime;
  myst.st_ctime = mip->INODE.i_ctime;
  printf("*****stat*****\n");
  printf("dev = %d\n", myst.st_dev);
  printf("ino = %d\n", myst.st_ino);
  printf("mode = %X\n", myst.st_mode);
  printf("nlink = %d\n", myst.st_nlink);
  printf("uid = %d\n", myst.st_uid);
  printf("gid = %d\n", myst.st_gid);
  printf("size = %d\n", myst.st_size);
  printf("atime = %s\n", ctime(&myst.st_atime));
  printf("mtime = %s\n", ctime(&myst.st_mtime));
  printf("ctime = %s\n", ctime(&myst.st_ctime));
  printf("***************\n");

  iput(mip);
  return 1;
}

// rwxrwxrwx = 0777
//|rwx|rwx|rwx|, e.g. 0644
// drwxrwxr-x = 0755
// i_mode =0x41ed(HEX-16) (OCT-8) or 0100 0001 1110 1101
//(0x41fd)
// The first 4 bits 0100 = directory, 0111 = regular file, 0110 = symbolic link
// The next 3 bits 000=ugs (user, group, other), 001=o, 010=g, 011=u
// d rwx(user) rwx(group) r-x(other user) <<<<<<d(directory)
// r(read for owner) w(write for owner) x(execute for owner)

// 0644(8)->110 100 100->
// imode = u16

// chmod dir1 100644(8) makes drwxr-xr-x to -rw-r--r--
// chmod file2 0777(8) makes -rw-r--r-- to rwxrwxrwx
int my_chmod()
{
  char *name;
  char buf[BLKSIZE];
  DIR *dp;
  int ino, dev;
  MINODE *mip;
  int mode;
  char mypath[256], newmode[256];
  sscanf(line, "%s %s %s", cmd, mypath, newmode);
  printf("chmod_file-1 %s %s %s\n", cmd, mypath, newmode);

  if (pathname[0] == '/')
  {
    dev = root->dev;
    mip = root;
    printf("chmod_file-2\n");
  }
  else
  {
    dev = running->cwd->dev;
    mip = running->cwd;
    printf("chmod_file-3\n");
  }
  ino = getino(pathname);
  if (ino == 0)
  {
    printf("File does not exist\n");
    printf("chmod_file-4\n");
    return 0;
  }

  mip = iget(dev, ino);
  /// mode = atoi(newmode);
  mode = strtol(newmode, NULL, 8);
  printf("newmode %s\n", newmode);
  printf("mode %d\n", mode);
  printf("mip->INODE.i_mode  %d\n", mip->INODE.i_mode);
  // mip->INODE.i_mode |= mode;
  mip->INODE.i_mode = mode;
  printf("mip->INODE.i_mode  %d\n", mip->INODE.i_mode);
  mip->dirty = 1;
  iput(mip);
  printf("chmod_file-5\n");
  return 1;
}
