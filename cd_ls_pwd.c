#include "functions.h"

/************* cd_ls_pwd.c file **************/
int cd()
{
  printf("cd: under construction READ textbook!!!!\n");
  printf("From running->cwd = mip[%d,%d]\n", running->cwd->dev, running->cwd->ino);

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
  printf("cd: dev = %d\n", dev);
  printf("cd: ino = %d\n", ino);
  MINODE *mip = iget(dev, ino);
  //(3). Verify mip->INODE is a DIR // return error
  // if not DIR
  if (mip->INODE.i_mode & 0x4000) // This somehow conflics with new device
  {
    printf("Is a DIR\n");
  }
  else
  {
    printf("Not a directory\n");
    iput(mip);
    printf("running->cwd = mip[%d,%d]\n", running->cwd->dev, running->cwd->ino);
    return 0;
  }
  //(4). iput(running->cwd); // release old cwd
  iput(running->cwd);
  //(5). running->cwd = mip; // change cwd to mip
  running->cwd = mip;
  printf("running->cwd = mip[%d,%d]\n", running->cwd->dev, running->cwd->ino);
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
