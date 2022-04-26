#ifndef FUNCTIONS
#define FUNCTIONS

// C Libs:
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ext2fs/ext2_fs.h>
#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

// USR
#include "type.h"
#include "globals.h"

/* util.c */
int get_block(int dev, int blk, char *buf);

int put_block(int dev, int blk, char *buf);

int tokenize(char *pathname);

MINODE *iget(int dev, int ino);

void iput(MINODE *mip);

int getino(char *pathname);

int findmyname(MINODE *parent, u32 myino, char myname[]);

int findino(MINODE *mip, u32 *myino);
int findino2(MINODE *mip, u32 *myino);

/* alloc.c */
int tst_bit(char *buf, int bit);
int decFreeInodes(int dev);
int decFreeBlocks(int dev);
int ialloc(int dev); // allocate an inode number from inode_bitmap
int balloc(int dev);
int incFreeInodes(int dev);
int incFreeBlocks(int dev);

/* dealloc.c */
int clr_bit(char *buf, int bit);
int my_idalloc(int dev, int ino);
int my_bdalloc(int dev, int ino);

/* cd_ls_pwd.c */
int cd();

int ls_file(MINODE *mip, char *name);

int ls_dir(MINODE *mip);

int ls();

int rpwd(MINODE *wd);

char *pwd(MINODE *wd);

/////
int enter_name(MINODE *pmip, int ino, char *name);
int my_kmkdir(MINODE *pip, char child[256]);
char *get_dirname(char *path);
int my_mkdir();
int my_kcreat(MINODE *pip, char child[256]);
int my_creat();
int rm_child(MINODE *pmip, char *name);
int my_rmdir();
int my_link();

int my_unlink();
int my_ksymlink(MINODE *pip, char child[256]);

int my_symlink();
int my_readlink();

int my_utime();
int my_stat();
int my_chmod();
/////open_close.c
int open_file();
int myopen(char *filename, int flags);
int my_truncate(MINODE *mip);
int close_file();
int myclose(int fd);
int my_lseek(int fd, int position);
int lseek_file();
int my_pfd();
int my_dup(int fd);
int my_dup2(int fd, int gd);
// read_write.c
int myread(int fd, char *buf, int nbytes);
int read_file();
int cat_file();
//
int write_file();
int mywrite(int fd, char buf[], int nbytes);
int my_cp();
int my_mv();
// mount_unmount.c
int mount();
int umount();
// mydebug.c
int mount_table();
int my_mip();
int my_dev();

#endif