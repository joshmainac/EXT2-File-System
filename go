# mkdisk: create a virtual disk diskimage

sudo dd if=/dev/zero of=diskimage bs=1024 count=1440
sudo mke2fs -b 1024 diskimage 1440

sudo mount diskimage /mnt

(cd /mnt; sudo rm -rf lost+found; sudo mkdir dir1 dir2 dir1/dir3; sudo touch file1 file2; ls -l)

sudo umount /mnt

sudo chmod 0777 diskimage

rm a.out 2> /dev/null

gcc *.c

./a.out

