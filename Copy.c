#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>


#define BUFFER 1024

int main(int argc, char *argv[]) {
	if (argc <= 2 || argc > 4) { //valid command will have name of program, two file dest arguments, & maybe a recursive flag
		printf("Usage: [OPTION] <Source> <Destination>\n");
		return 0;
	}
	int toCopy;
	int copyTo;
	int hasToCopyAssigned = 0; //0 if false, 1 if true
	int recursiveFlag = 0;
	//this loop is used to assign and dispatch arguments properly 
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-r") == 0) { //checks if argument is recursive flag
			recursiveFlag = 1;
		} else {
			if (hasToCopyAssigned == 0) {
				toCopy = i;
				hasToCopyAssigned = 1;
			} else {
				copyTo = i;
			}
		}
	}
	if (recursiveFlag == 1) {
		copyFileRecursive(argv[toCopy], argv[copyTo]);
		return 0;
	}
	copyFile(argv[toCopy], argv[copyTo]);
	return 0;
}

void copyFileRecursive(char *dirToCopy, char *dirCopyTo) {
	printf("Entered recursive call!\n------------------\n");
	printf("Current parameters are: "); printf(dirToCopy); printf(" and "); printf(dirCopyTo); printf("\n");
	char copyPath[1000];
	char toPath[1000];

	struct dirent *dirPt;
	struct dirent *dirPtDest;
	//Open the source directory which we will copy files from and recursively enter subdirectories
	DIR *dir = opendir(dirToCopy); if (!dir) {printf("ERROR: Could not open dir to copy from!");}
	//Read the source directory
	while ((dirPt = readdir(dir)) != NULL) { //dirPt represents each entry in the source directory
		//We don't care about . or .. directories, only execute further code if our dir
		//entry is not . or ..
		if (strcmp(dirPt->d_name, "..") != 0 && strcmp(dirPt->d_name, ".") != 0) {
			//Identify whether the directory entry (dirPt) is a file or subdirectory
			DIR *checkDir; //Validation variable
			//Concat source directory & current directory entry
			char checkPath[1024];
			strcpy(checkPath, dirToCopy);
			strcat(checkPath, "/");
			strcat(checkPath, dirPt->d_name);
			checkDir = opendir(checkPath);
			if (checkDir) { //If opening checkPath was successful, the dirPt points to a directory
				printf(checkPath);
				printf(" is a directory!\n");
				//Construct recursive call to enter subdirectory
				strcpy(copyPath, dirToCopy); strcpy(toPath, dirCopyTo);

				strcat(copyPath, "/");
				strcat(copyPath, dirPt->d_name); //copyPath now represents subdirectory to copy from
				
				strcat(toPath, "/");
				strcat(toPath, dirPt->d_name); //toPath now represents subdirectory to copy to
				
				printf("Attempting recursive call with parameters: ");
				printf(copyPath); printf(" and "); printf(toPath); printf("\n");
				
				//toPath subdirectory probably DNE, create there if DNE
				printf("Attempting mkdir with path: "); printf(toPath); printf("\n");
				mkdir(toPath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH); //rws for owner, grp, rs other	

				copyFileRecursive(copyPath, toPath);
			} else { //If opening checkPath was unsuccessful, the dirPt points to a file
				//Construct path for file to copy, & path to copy file to
				strcpy(copyPath, dirToCopy); strcpy(toPath, dirCopyTo);
									
				strcat(copyPath, "/");
				strcat(copyPath, dirPt->d_name);

				strcat(toPath, "/");
				strcat(toPath, dirPt->d_name);

				printf("Attempting copyFile() call with parameters: ");
				printf(copyPath); printf(" and "); printf(toPath); printf("\n");
				copyFile(copyPath, toPath);
			}
			
		}
	}



}

//void copyFileRecursive(char *dirToCopy/*[]*/, char *dirCopyTo/*[]*/) {
//	//loop through directory pointers
//	printf("Entered recursive call!\n");
//	char path[1000];
//	struct dirent *dirPt;
//	struct dirent *dirPtDest;
//	DIR *dir = opendir(dirToCopy);
//	//open dir for dest - add
//	DIR *checkDir;
//	if (!dir) {printf("ERROR: Could not open directory to copy from!");}
//	while ((dirPt = readdir(dir)) != NULL) {
//		printf("Entered directory loop pointing to: ");
//		printf(dirPt->d_name);
//		printf("\n");
//		strcat(dirToCopy, dirPt->d_name);
//		checkDir = opendir(dirToCopy/*dirPt->d_name*/);	
//		if ((checkDir)/*) != NULL*/) { //is a directory
//			if (strcmp(dirPt->d_name, ".") != 0 && strcmp(dirPt->d_name, "..") != 0) {
//				printf("Pointer ");
//				printf(dirPt->d_name);
//				printf(" is a directory!\n");
//				//strcat(str1, str2)
//				strcat(dirCopyTo, "/");
//				strcat(dirCopyTo, dirPt->d_name);
//				printf("New directory to copy to is:  ");
//				printf(dirCopyTo);
//				printf("\n");
//				copyFileRecursive(dirPt->d_name, dirCopyTo/*dirCopyTo+dirPt*/); //access d_name char array property of dirent struct
//			}
//		} else { //is a file
//			printf("Identified a file named: ");
//			printf(dirPt->d_name);
//			printf("\n");
//		}
//	}		
//	// - if file, copy(source, dest)
//
//	// - if dir, copyFileRecursive(dirPt, dest+dirPt)
//}

void copyFile(char toCopy[], char copyTo[]) {
	ssize_t numberRead; //represents size of allocated block of mem
	int fileDestIn = open(toCopy, O_RDONLY);
	if (fileDestIn == -1) {printf("ERROR: Could not open file to copy!\n");}
	
	//open in rw mode, create if file DNE, rw perms for usr, group, and other
	int fileDestOut = open(copyTo, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fileDestOut == -1) {printf("ERROR: Could not open file to write to!\n");}

	char buf[BUFFER];

	/*
 	read() returns number of bytes read & placed in buffer.  0 means end of file.
	This means we can check for when we are at the end of the file & prevent
	writing any excess bytes to the output file...  
 	*/

	while ((numberRead = read(fileDestIn, buf, BUFFER)) > 0) {
		if (write(fileDestOut, buf, numberRead) != numberRead) {printf("ERROR: Failed to write to output file...\n");}
	}

	if (close(fileDestIn) == -1) {printf("ERROR: Could not close file to copy!\n");}	
	if (close(fileDestOut) == -1) {printf("ERROR: Could not close copied file!\n");}
}
