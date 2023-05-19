# xv6 Operating System

## [project specification](OS-Domaći2.pdf)

## Adding file recovery support

Xv6 has been changed to support best effort recovery. This only works for files. Directory recovery is not supported.
A file can only be recovered if its integrity is untouched. This means that none of the file's memory blocks are used by another file or directory.<br/>

To make this task possible, I've made two significant changes to xv6.<br/>
- The first is to keep track of the memory blocks that are part of a file by adding array[memory block address] to a file structure.<br/>
- Second, each file in the directory is assigned a flag indicating whether or not a file is deleted by including the ***del*** attribute in the dirent structure.<br/>

By default, when a file is deleted in xv6, all its contents disappear and the busy flag is set to zero, indicating that a file structure can be used by other directories to store data. I have changed this so that when a file is deleted, all of its data remains on disk, but the busy flag is set to 0 and the del flag is set to 1. This way the data of the file is preserved, but the file structure is available for reuse.<br/>

Additional changes to the xv6 OS:

1. Two system calls :
      
    -   **int lsdel(char \*path, char \*result)**<br/>
        Lists all deleted files from directory.<br/>
        @*param \*path* - path to the directory to be listing from.<br/>
        @*param \*result* - names of all deleted files will be pushed in the result.<br/>
        Returning value is the number of deleted files in directory or -1 if the path is wrong.<br/>

    -   **int rec(char \*path)**<br/>
        Tries ''best effort'' recovery of the file.<br/>      
        @*param \*path* - path to the file to be recovered.<br/>
        Returning value is : 0-successful recovery, 1-wrong path, 2-file not found in the directory, 3-file structure is used for somethig else, 4-some of the file's memory blocks is used for something else.<br/>

2. Three user programs:

    -   **lsdel [path]**<br/>
        Prints deleted files from the directory given by the path. If the path is omited, then it considers current directory by default.<br/>
    
    -   **rec \<path\>**<br/>
        Tries to recover a file given by the path or reports the error if it's not possible.<br/>
    
    -   **writter \<filename\> \<numberOfBytes\>**<br/>
        Creates file with a file name \<filename\> and size of \<numberOfBytes\>.<br/>

---

- To start the program type commands:
1. Open the terminal in project's directory<br/>
2. Call command ***'make clean'***<br/>
3. Then call command ***'make qemu'***<br/>

The xv6 operating system should start at this point, and the QEMU window should be displayed.<br/><br/>

---

- Simple example how to create, delete and recover a file:
1. type ***'writer a 500'***<br/>
2. type ***'rm a'***<br/>
3. type ***'rec a'***<br/><br/>

- Example of an error message that the file structure where the deleted file was stored is used for something else:
1. type ***'cd home'***<br/>
2. type ***'writer a 500'***<br/>
3. type ***'rm a'***<br/>
4. type ***'cd ..'***<br/>
5. type ***'writer b 500'***<br/>
6. type ***'cd home'***<br/>
7. type ***'rec a'***<br/>
The error occurred because after deleting file 'a' we created file 'b' in another directory, which now occupies the file structure where file 'a' was before.

- Example of an error message stating that some of the blocks of the deleted file are used for something else:
1. type ***'cd home'***<br/>
2. type ***'writer a 10'***<br/>
3. type ***'writer b 10'***<br/>
3. type ***'rm a b'***<br/>
4. type ***'cd ..'***<br/>
5. type ***'writer c 1500'***<br/>
6. type ***'cd home'***<br/>
7. type ***'rec b'***<br/>
The error occurred because after deleting files 'a' and 'b', we created file 'c' with a size of 1500 bytes, which overwrites the memory blocks of both files 'a' and 'b'.
Note that the file 'c' occupies the file structure where the file 'a' was before.<br/><br/>






