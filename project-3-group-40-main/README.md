# **Project 3: FAT32 File System**

## Description

- For this project, we will design and implement a simple, user-space, shell-like utility that is capable of interpreting a FAT32 file system image.
- Our program must understand the basic commands to manipulate the given file system image, must not corrupt the file system image, and should be robust.
- We may not reuse kernel file system code, and we may not copy code from other file system utilities.
- We are tasked with writing a program that supports the following file system commands to a FAT32 image file.
- For good modular coding design, we will implement each command in one or more separate functions (e.g., for write we may have several shared lookup functions, an update directory entry function, and an update cluster function).
- Our program will take the image file path name as an argument and will read and write to it according to the different commands.
- We can check the validity of the image file by mounting it with the loop back option and using tools like Hexedit.
- We will also need to handle various errors.
- When we encounter an error, we should print a descriptive message (e.g., when cd’ing to a nonexistent file we can do something like “Directory not found: foo”).
- Furthermore, our program must continue running, and the state of the system should remain unchanged as if the command was never called (i.e., don’t corrupt the file system with invalid data).

## Group Members
1. **Souhail Marnaoui**:
  > sm22cb@fsu.edu
3. **Iskandar Verdiyev**:
  > iv22d@fsu.edu
5. **Panayoti Kourkoumelis**:
  > pk22j@fsu.edu

## Division of Labor
### Part 1: Mount the Image File
**Details**:
- The user will need to mount the image file through command line arguments:
```bash
./filesys [FAT32 ISO] ->
```
- You should read the image file and implement the correct structure to store the format of FAT32 and
navigate it.
- You should close out the program and return an error message if the file does not exist.
- The user will then be greeted with a standard shell that will accept user input.
- Your terminal should look like this:
```
[NAME_OF_IMAGE]/[PATH_IN_IMAGE]/>
```
- The following commands will need to be implemented:

```info ->```
  - Parses the boot sector. Prints the field name and corresponding value for each entry, one per line (e.g., Bytes Per Sector: 512).
  - The fields you need to print out are:
    - bytes per sector
    - sectors per cluster
    - root cluster
    - total # of clusters in data region
    - Number of entries in one fat
    - size of image (in bytes)

```exit ->```
  - Safely closes the program and frees up any allocated resources.

**Assigned to**:
> Souhail Marnaoui, Iskandar Verdiyev


### Part 2:  Navigation
**Details**:
- You will need to create these commands that will allow the user to navigate the image.

```cd [DIRNAME] ->```
  - Changes the current working directory to [DIRNAME]. ‘.’ and ‘..’ are valid directory names to cd.
    - Your code will need to maintain the current working directory state.
    - Print an error if [DIRNAME] does not exist or is not a directory.

```ls ->```
  - Print the name filed for the directories within the current working directory including the “.” And “..” directories.
    - For simplicity, you may print each of the directory entries on separate lines and no colors required.


**Assigned to**:
> Souhail Marnaoui, Panayoti Kourkoumelis


### Part 3: Create
**Details**:
- You will need to create commands that will allow the user to create files and directories.

```mkdir [DIRNAME] ->```
  - Creates a new directory in the current working directory with the name `[DIRNAME]`.
    - Need to create ‘.’ and ‘..’ inside this newly created directory.
    - Print an error if a directory/file called `[DIRNAME]` already exists.

```creat [FILENAME] ->```
  - Creates a file in the current working directory with a size of 0 bytes and with a name of `[FILENAME]`. The `[FILENAME]` is a valid file name, not the absolute path to a file.
    - Print an error if a directory/file called `[FILENAME]` already exists.

### Part 4: Read
**Details**:
- You will need to create commands that will read from opened files. Create a structure that stores which files are opened.

```open [FILENAME] [FLAGS] ->```
  - Opens a file named `[FILENAME]` in the current working directory. A file can only be read from or written to if it is opened first. You will need to maintain some data structure of opened files and add `[FILENAME]` to it when open is called. `[FLAGS]` is a flag and is only valid if it is one of the following (do not miss the ‘-‘ character for the flag):
    - r - read-only.
    - w - write-only.
    - rw - read and write.
    - wr - write and read.
  - Initialize the offset of the file at 0. Can be stored in the open file data structure along with other info.
  - Print an error if the file is already opened, if the file does not exist, or an invalid mode is used.

```close [FILENAME] ->```
  - Closes a file [FILENAME] in current working directory.
    - Needs to remove the file entry from the open file data structure.
    - Print an error if the file is not opened, or if the file does not exist in current working directory.

```lsof ->```
  - Lists all opened files.
    - Needs to list the index, file name, mode, offset, and path for every opened file.
    - If no files are opened, notify the user.

```lseek [FILENAME] [OFFSET] ->```
  - Set the offset (in bytes) of file `[FILENAME]` in current working directory for further reading or writing.
    - Store the value of `[OFFSET]` (in memory) and relate it to the file `[FILENAME]`.
    - Print an error if file is not opened or does not exist.
    - Print an error if `[OFFSET]` is larger than the size of the file.

```read [FILENAME] [SIZE] ->```
  - Read the data from a file in the current working directory with the name `[FILENAME]`, and print it out.
    - Start reading from the file’s stored offset and stop after reading `[SIZE]` bytes.
    - If the offset + `[SIZE]` is larger than the size of the file, just read until end of file.
    - Update the offset of the file to offset + `[SIZE]` (or to the size of the file if you reached the end of the file).
    - Print an error if `[FILENAME]` does not exist, if `[FILENAME]` is a directory, or if the file is not opened for reading.

**Assigned to**:
> Iskandar Verdiyev, Panayoti Kourkoumelis


### Part 5: Update
**Details**:
- You will need to implement the functionality that allows the user to write to a file.

```write [FILENAME] [STRING] ->```
  - Writes to a file in the current working directory with the name `[FILENAME]`.
    - Start writing at the file’s offset and stop after writing `[STRING]`.
    - `[STRING]` is enclosed in "". You do not need to wrong about "" in `[STRING]`.
    - If offset + size of `[STRING]` is larger than the size of the file, you will need to extend the length of the file to at least hold the data being written.
    - Update the offset of the file to offset + size of `[STRING]`.
    - Print an error if `[FILENAME]` does not exist, if `[FILENAME]` is a directory, or if the file is not opened for writing.

**Assigned to**:
> Souhail Marnaoui, Panayoti Kourkoumelis


### Part 6: Delete
**Details**:
- You will need to implement the functionality that allows the user to delete a file/directory.

```rm [FILENAME] ->```
  - Deletes the file named `[FILENAME]` from the current working directory.
    - This means removing the entry in the directory as well as reclaiming the actual file data.
    - Print an error if `[FILENAME]` does not exist or if the file is a directory or if it is opened.

```rmdir [DIRNAME] ->```
  - Remove an empty directory by the name of `[DIRNAME]` from the current working directory.
    - This command alone can only be used on an empty directory (if a directory only contains ‘.’ and ‘..’, it is an empty directory).
    - Make sure to remove the entry from the current working directory and to remove the data `[DIRNAME]` points to.
    - Print an error if the `[DIRNAME]` does not exist, if `[DIRNAME]` is not a directory, or `[DIRNAME]` is not an empty directory or if a file is opened in that directory.

**Assigned to**:
> Souhail Marnaoui, Iskandar Verdiyev


## File Listing
- Before `Make`:
```
root/
├── inc/
|    ├── BPB.h
|    ├── DIR.h
|    ├── mount.h
|    ├── run.h
|    ├── cd.h
|    ├── ls.h
|    ├── mkdir.h
|    ├── open.h
|    ├── close.h
|    ├── lsof.h
|    ├── lseek.h
|    ├── read.h
|    ├── write.h
|    ├── rm.h
|    ├── rmdir.h
|    └──
├── src/
|    ├── main.c
|    ├── mount.c
|    ├── run.c
|    ├── cd.c
|    ├── ls.c
|    ├── mkdir.c
|    ├── open.c
|    ├── close.c
|    ├── lsof.c
|    ├── lseek.c
|    ├── read.c
|    ├── write.c
|    ├── rm.c
|    ├── rmdir.c
|    ├── 
|    └── 
├── Makefile
└── README.md
```

- After `Make`:
```
root/
├── bin/
|    └── filesys 
├── inc/
|    ├── BPB.h
|    ├── DIR.h
|    ├── mount.h
|    ├── run.h
|    ├── cd.h
|    ├── ls.h
|    ├── mkdir.h
|    ├── open.h
|    ├── close.h
|    ├── lsof.h
|    ├── lseek.h
|    ├── read.h
|    ├── write.h
|    ├── rm.h
|    ├── rmdir.h
|    └──
├── obj/
|    ├── [*.o]
|    ├── [*.o]
|    ├── [*.o]
|    └── [*.o]
├── src/
|    ├── main.c
|    ├── mount.c
|    ├── run.c
|    ├── cd.c
|    ├── ls.c
|    ├── mkdir.c
|    ├── open.c
|    ├── close.c
|    ├── lsof.c
|    ├── lseek.c
|    ├── read.c
|    ├── write.c
|    ├── rm.c
|    ├── rmdir.c
|    ├── 
|    └── 
├── Makefile
└── README.md
```

## How to Compile & Execute

### Requirements
- **Compiler**: GCC 12.x

### Compilation
- This will create the necessry directories, and compile the project:
```bash
make
```

### Execution
- This will run the FAT32 interpreter using the provided `*.img` file:
```bash
bin/filesystem fat32.img
```

## Bugs
- No Known bugs as of latest commit
