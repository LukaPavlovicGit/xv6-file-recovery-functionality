# xv6 Operating System
## Adding interprocess communication

Xv6 modifed to support ''best effort'' recovery. It only works for files. Directory recovery is not supported.<br/>
File is recoverable only if it's integrity remained untouched. That means that neither of file's memory blocks is used by other file or directory.<br/>

To make this task possible i've made two major changes of xv6 OS.<br/>
- First is to keep track of memory blocks which are part of a file by adding array[memory block address] in a file structure.<br/>
- Second is to assign a flag to each file in the directory which indicates whether a file is deleted or not by adding ***del*** attribute in the dirent structure.<br/>

By default, when file is deleted in xv6 all of it's content dissappear and set busy flag to zero which indicates that a file structure can be used by other directories to store data. I've changed that in a such way that if file is deleted all of file's data remain on disk as it was, but to set busy flag to 0 and del flag to 1. Doing this we've preserved file's data, but made file structure available for reuse.<br/>

Additional changes to the xv6 OS:

1. Two system calls :
      
    -   **int lsdel(char \*path, char \*result)**<br/>
        Lists all deleted files from directory.<br/>
        @*param path* - path to the directory to be listing from.<br/>
        @*param result* - names of all deleted files will be pushed in the result.<br/>
        Returning value is the number of deleted files in directory or -1 if the path is wrong.<br/>

    -   **int rec(char \*path)**<br/>
        Tries ''best effort'' recovery of the file.<br/>      
        @*param path* - path to the file to be recovered.<br/>
        Returning value is : 0-successful recovery, 1-wrong path, 2-file not found in the directory, 3-file structure is used for somethig else, 4-some of the file's memory blocks is used for something else.<br/>

2. Three user programs:

    -   **lsdel [path]**<br/>
        Prints deleted files from the directory given by the path. If the path is omited, then it considers current directory by default.<br/>
    
    -   **rec \<path\>**<br/>
        Tries to recover a file given by the path or reports the error if it's not possible.<br/>
    
    -   **writter \<filename\> \<numberOfBytes\>**<br/>
        Creates file with a file name \<filename\> and size of \<numberOfBytes\>.<br/>

---

To start the program type next commands:
1. Open the terminal in project's directory<br/>
2. Call command ***'make clean'***<br/>
3. Then call command ***'make qemu'***<br/>
The xv6 operating system should be started at this point, and QEMU window should be displayed<br/><br/>

---

Simple example how to create, delete and recover a file:<br/>
1. type ***'writer a 500'***<br/>
2. type ***'rm a'***<br/>
3. type ***'rec a'***<br/>

Example how to generate error that file structure in which the deleted file have been stored is used for something else:<br/>
1. type ***'cd home'***<br/>
2. type ***'writer a 500'***<br/>
3. type ***'rm a'***<br/>
4. type ***'cd ..'***<br/>
5. type ***'writer b 500'***<br/>
6. type ***'cd home'***<br/>
7. type ***'rec a'***<br/>
Error occured because after we deleted file 'a', we've created file 'b' in different directory who's now occupying file structure where the file 'a' been before.<br/><br/>

Example how to generate error that some of the deleted file's blocks are used for something else:
1. type ***'cd home'***<br/>
2. type ***'writer a 10'***<br/>
3. type ***'writer b 10'***<br/>
3. type ***'rm a b'***<br/>
4. type ***'cd ..'***<br/>
5. type ***'writer c 1500'***<br/>
6. type ***'cd home'***<br/>
7. type ***'rec b'***<br/>
Error occured because after we deleted files 'a' and 'b', we've created file 'c' sized of 1500 bytes which overrides memory blocks of both files 'a' and 'b'.<br/>
Notice that the file 'c' is occupying file structure where the file 'a' been before.<br/><br/>






