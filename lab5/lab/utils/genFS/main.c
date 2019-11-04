#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>
#include "utils.h"
#include "data.h"
#include "func.h"

/*
int main(int argc, char *argv[]) {
	char driver[NAME_LENGTH];
	char rootDirPath[NAME_LENGTH];
	char srcFilePath[NAME_LENGTH];
	char destDirPath[NAME_LENGTH];
	char destFilePath[NAME_LENGTH];

	stringCpy("fs.bin", driver, NAME_LENGTH - 1);
	stringCpy("/", rootDirPath, NAME_LENGTH - 1);
	stringCpy("./test", srcFilePath, NAME_LENGTH - 1);
	stringCpy("/doc", destDirPath, NAME_LENGTH - 1);
	stringCpy("/doc/test", destFilePath, NAME_LENGTH - 1);
	
	format(driver, SECTOR_NUM, SECTORS_PER_BLOCK);
	mkdir(driver, destDirPath);
	ls(driver, rootDirPath);
	cp(driver, srcFilePath, destFilePath);
	ls(driver, destDirPath);
	ls(driver, destFilePath);
	//cat(driver, destFilePath);
	rm(driver, destFilePath);
	ls(driver, destDirPath);
	ls(driver, rootDirPath);
	rmdir(driver, destDirPath);
	ls(driver, rootDirPath);

	return 0;
}
*/

int main(int argc, char *argv[]) {
	char driver[NAME_LENGTH];
	char srcFilePath[NAME_LENGTH];
	char destFilePath[NAME_LENGTH];

	stringCpy("fs.bin", driver, NAME_LENGTH - 1);
	
    // STEP 1
    // TODO: build file system of os.img, see lab5 4.3.
    // All functions you need have been completed

	char rootDirPath[NAME_LENGTH];
	char bootDirPath[NAME_LENGTH];
	char devDirPath[NAME_LENGTH];
	char usrDirPath[NAME_LENGTH];
	char initrdFilePath[NAME_LENGTH];
	char stdinFilePath[NAME_LENGTH];
	char stdoutFilePath[NAME_LENGTH];

	stringCpy("/", rootDirPath, NAME_LENGTH - 1);  //Paste name to strings
	stringCpy("/boot", bootDirPath, NAME_LENGTH - 1);
	stringCpy("/dev", devDirPath, NAME_LENGTH - 1);
	stringCpy("/usr", usrDirPath, NAME_LENGTH - 1);
	stringCpy("/boot/initrd", initrdFilePath, NAME_LENGTH - 1);
	stringCpy("/dev/stdin", stdinFilePath, NAME_LENGTH - 1);
	stringCpy("/dev/stdout", stdoutFilePath, NAME_LENGTH - 1);
	//stringCpy("/boot/initrd", destFilePath, NAME_LENGTH - 1);
	stringCpy("uMain.elf", srcFilePath, NAME_LENGTH - 1);

	format(driver, SECTOR_NUM, SECTORS_PER_BLOCK); //Format fs.bin, initialization 

	mkdir(driver, rootDirPath); // Create Directory
	mkdir(driver, bootDirPath);
	mkdir(driver, devDirPath);
	mkdir(driver, usrDirPath);
	//touch(driver, initrdFilePath);
	touch(driver, stdinFilePath); // Create file
	touch(driver, stdoutFilePath);
	cp(driver, srcFilePath, initrdFilePath); // Copy uMain.elf(user programme) to fs.bin

    ls(driver, "/");
    ls(driver, "/boot/");
    ls(driver, "/dev/");
    ls(driver, "/usr/");
    
    /** output:
    ls /
    Name: boot, Type: 2, LinkCount: 1, BlockCount: 1, Size: 1024.
    Name: dev, Type: 2, LinkCount: 1, BlockCount: 1, Size: 1024.
    Name: usr, Type: 2, LinkCount: 1, BlockCount: 0, Size: 0.
    LS success.
    8185 inodes and 3052 data blocks available.
    ls /boot/
    Name: initrd, Type: 1, LinkCount: 1, BlockCount: 14, Size: 13400.
    LS success.
    8185 inodes and 3052 data blocks available.
    ls /dev/
    Name: stdin, Type: 1, LinkCount: 1, BlockCount: 0, Size: 0.
    Name: stdout, Type: 1, LinkCount: 1, BlockCount: 0, Size: 0.
    LS success.
    8185 inodes and 3052 data blocks available.
    ls /usr/
    LS success.
    8185 inodes and 3052 data blocks available.
	*/
    
	return 0;
}
