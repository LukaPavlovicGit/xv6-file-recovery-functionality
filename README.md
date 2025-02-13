## Adding File Recovery Support to the XV6 Operating System


# [Video Demonstration](https://drive.google.com/file/d/1xie6yoz6gtYFC3n2xgE4xUmpk_Tbwmpo/view?usp=drive_link)
# [Project Specification](OS-Domaći2.pdf)

Xv6 has been modified to support best-effort file recovery. This feature only works for files; directory recovery is not supported. A file can only be recovered if its integrity remains intact, meaning that none of its data blocks have been reused by another file or directory.<br/>

To make this possible, I introduced two significant changes to XV6:<br/>
1. **Track File Blocks:**  
   Each file now has an array of its memory block addresses, stored in its file structure.  
2. **Deleted Flag in Directory Entries:**  
   Each directory entry has a `del` flag indicating whether the file is deleted.  

By default, when a file is deleted in XV6, all its content is removed, and the busy flag is cleared (set to 0). Consequently, the file structure can be reused for storing data. I changed this behavior so that when a file is deleted, all of its data remains on disk, but the busy flag is set to 0, and the `del` flag is set to 1. This way, the file’s data is preserved while the file structure remains available for reuse.<br/>

### Additional Changes to the XV6 OS

1. **Two System Calls**  
    - **`int lsdel(char *path, char *result)`**  
      Lists all deleted files in a given directory.  
      - **`path`**: Path to the directory to search.  
      - **`result`**: A buffer where names of all deleted files are appended.  
      - **Return Value**: The number of deleted files in the directory, or **-1** if the path is invalid.  

    - **`int rec(char *path)`**  
      Attempts a "best-effort" recovery of a deleted file.  
      - **`path`**: Path to the file to be recovered.  
      - **Return Value**:  
        - **0** = Recovery successful  
        - **1** = Invalid path  
        - **2** = File not found in the directory  
        - **3** = File structure is already in use by something else  
        - **4** = Some of the file’s blocks are used by another file or directory  

2. **Three User Programs**  
    - **`lsdel [path]`**  
      Prints deleted files from the specified directory. If no path is provided, it defaults to the current directory.  
      
    - **`rec <path>`**  
      Attempts to recover a file at the specified path, or prints an error message if recovery is not possible.  
      
    - **`writer <filename> <numberOfBytes>`**  
      Creates a file named `<filename>` of size `<numberOfBytes>`.  

---

### How to Run

1. Open a terminal in the project’s directory.  
2. Run **`make clean`**.  
3. Run **`make qemu`**.  

At this point, the XV6 operating system should start, and a QEMU window will be displayed.<br/>

---

### Simple Example: Create, Delete, and Recover a File

1. Type **`writer a 500`**  
2. Type **`rm a`**  
3. Type **`rec a`**  

---

### Example: File Structure Reused

1. Type **`cd home`**  
2. Type **`writer a 500`**  
3. Type **`rm a`**  
4. Type **`cd ..`**  
5. Type **`writer b 500`**  
6. Type **`cd home`**  
7. Type **`rec a`**  

The error occurs because, after deleting file **`a`**, file **`b`** was created in another directory, which reused the file structure previously occupied by **`a`**.

---

### Example: Overwritten Memory Blocks

1. Type **`cd home`**  
2. Type **`writer a 10`**  
3. Type **`writer b 10`**  
4. Type **`rm a b`**  
5. Type **`cd ..`**  
6. Type **`writer c 1500`**  
7. Type **`cd home`**  
8. Type **`rec b`**  

The error occurs because, after deleting files **`a`** and **`b`**, we created **`c`** (1500 bytes), which overwrote the memory blocks used by both **`a`** and **`b`**. Note that **`c`** also reused the file structure that **`a`** occupied.
