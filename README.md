# EXT2-File-System

* This project involves the implementation of an EXT2 file system, which is used in the Linux operating system, in the programming language C.
* The file system includes functionality for creating and modifying directories, files, and links, as well as reading and writing to files.
* The project also involves optimizing reading and writing to indirect and double indirect blocks in order to improve performance.
* The file system includes a mount and unmount function, and requires modifications to existing functions in order to support cross mounting points.
* Disk I/O is performed using virtual disks that are simulated by Linux files, and accessed using Linux's read() and write() functions on a block size basis. These virtual disks can be created using Linux's mke2fs tool.
* The file system operates using processes, with initial processes for a superuser and an ordinary user. The file system executes commands in order to perform file operations, including required commands for basic file system functionality (mounting and unmounting, creating and modifying directories and files, linking and unlinking) and optional minor commands (stat, chmod, utime)...


## Requirements:

- Add your names to this file.
- Ensure your code can be ran with a `mk` script.
- include a working diskimage in your submission.

## Details:

- All final project submissions will be done in this project. Submit Level one, two and three here on their respective due dates.
  - We will clone them at the normal submission time.
- Demo will be a litte different, join the zoom call and be prepared to share your screen and run your code.
- Sign up (individuals AND partners) at this google sheet link. We will be making the demo schedule Wednesday at 5p.
  - https://docs.google.com/spreadsheets/d/1YDyWAbeTQYjvdJ4y-827NOEqvoHtWIJmwT1fuYInFiE/edit?usp=sharingss
