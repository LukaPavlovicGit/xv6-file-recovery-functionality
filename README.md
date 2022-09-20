# xv6 Operating System
## Adding interprocess communication

Xv6 modifed to support ''best effort'' recovery. It only works for files. Directory recovery is not supported.<br/>
File is recoverable only if it's integrity remained untouched. That means that neither of file's memory blocks is used by other file or directory.<br/>

To make this task possible i've made two major changes of xv6 OS.<br/>
- First is to keep track of memory blokcs which are part of a file by adding array[memory block address] in a file structure.<br/>
- Second is to assign a *del* flag to each file in the directory which indicates whether a file is deleted or not by adding del flag dirent structure.<br/>

By default, when file is deleted in xv6 all of it's content dissappear and set busy flag to zero which indicates whether a file structure can be used by other directories to store data. I've changed that in a such way that if file is deleted all of a file's data remain on disk as it was, but to set busy flag to 0 and del flag to 1.<br/>

Additional changes to the xv6 OS:

1. Two system calls :
      
    -   **int lsdel(char \*path, char \*result)**<br/>
        Lists all deleted files from directory.<br/>
        *param path* - path to the directory to be listing from.<br/>
        *param result* - names of all deleted files will be pushed in the result.<br/>
        Returning value is the number of deleted files in directory or -1 if the path is wrong.<br/>

    -   **int rec(char \*path)**<br/>
        Tries ''best effort'' recovery of the file.  <br/>      
        *param path* - path to the file to be recovered.<br/>
        Returning value is : 0-successful recovery, 1-wrong path, 2-file not found in the directory, 3-file structure is used for somethig else, 4-some of the file's memory blocks is used for something else.<br/>

2. Three user programs:

    -   **lsdel [path]**<br/>
        Prints deleted files from the directory given by the path. If path is omited, then it considers current directory.<br/>
    
    -   **rec \<path\>**<br/>
        Tries to recover a file given by the path or reports the error if it's not possible.<br/>
    
    -   **writter \<filename\> \<numberOfBytes\>** <br/>
        Creates file with a file name \<filename\> and size of \<numberOfBytes\>.<br/>


