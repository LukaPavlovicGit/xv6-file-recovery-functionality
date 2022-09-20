# xv6 Operating System
## Adding interprocess communication

Xv6 modifed to support ''best effort'' recovery. It only works for files. Directory recovery is not supported.
File is recoverable only if it's integrity remained untouched. That means that neither of file's memory blocks is used by other file or directory.

To make this task possible i've made two major changes of xv6 OS.
First is to keep track of memory blokcs which are part of a file by adding array[memory block address] in a file structure.<
Second is to assign a *del* flag to each file in the directory which indicates whether a file is deleted or not by adding del flag dirent structure.

By default, when file is deleted in xv6 all of it's content dissappear and set busy flag to zero which indicates whether a file structure can be used by other directories to store data. I've changed that in a such way that if file is deleted all of a file's data remain on disk as it was, but to set busy flag to 0 and del flag to 1.

Additional changes to the xv6 OS.

1. Two system calls :

    -   int lsdel(char *path, char * result)
        Lists all deleted files from directory.
        ***param path*** - path to the directory to be listing from.
        ***param result*** - names of all deleted files will be stored in the result
        Returning value is the number of deleted files in directory or -1 if the path is wrong.

    -   int rec(char *path)
        Tries ''best effort'' recovery of the file.
        ***param path*** - path to the file to be recovered.
        Returning value is : 0 - successful recovery, 1 - wrong path, 2 - file not found in the directory, 3 - file structure is used for somethig else, 4 - some of the file's memory blocks is used for something else.\\n

2. Three user programs:

    -   lsdel [path]
        Prints deleted files from the directory given by the path. If path is omited, then it considers current directory.
    
    -   rec <path>
        Tries to recover a file given by the path or reports the error if it's not possible.
    
    -   writter <filename> <numberOfBytes>
        Creates file with a file name <filename> and size of <numberOfBytes>. 


BUILDING AND RUNNING XV6
To build xv6 on an x86 ELF machine (like Linux or FreeBSD), run
"make". On non-x86 or non-ELF machines (like OS X, even on x86), you
will need to install a cross-compiler gcc suite capable of producing
x86 ELF binaries (see https://pdos.csail.mit.edu/6.828/).
Then run "make TOOLPREFIX=i386-jos-elf-". Now install the QEMU PC
simulator and run "make qemu".
