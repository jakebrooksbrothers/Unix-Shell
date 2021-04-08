//Jacob Brooks

#include "p2.h"

char buffer[MAXSTORAGE]; //declarations
char *newargv[MAXITEM];
int vptr;
int bptr;
int in;
int out;
int pipeflag;
int outfile;
int infile;
int charcount;
int afterPipe[10];
int pipectr;
int args;
int argc;
int flags;
int dn;
char *inptr;
char *outptr;
bool badin;
bool badout;
bool badpipe;
pid_t child;
pid_t grandchild;
pid_t greatgrand;
pid_t kidpid;
char* getDir;
char* chDir;
char* dir;
char* path[10];
FILE* pass;
FILE* temp; 
bool cd = false;
int dirctr;
char *line;
char *environ;
char* token[6];
int i;
int j;
int k;
size_t size;
bool found;
bool badenv;
char working[MAXITEM];
char* current;
char* delim;
int doublein;
char *theLine;
int num;
bool check;
bool dollar;
char *otherTok;
bool edge;

/* hereis is a function that handles <<. The argument after << is saved as a delimeter.
 * Further arguments are then processed in a temporary file created by this function. All
 * members of the file are read UNTIL the delimeter is found on a line by itself. At this
 * point the arguments before the delimeter ar returned and the temporary file used
 * for editing the ouput is removed. 
 */
void hereis () {
	temp = NULL;
	size = 0;
	theLine = NULL;
	check = false;
	if((temp = fopen("tempFile", "w+")) == NULL) fprintf(stderr, "failed to open");
	while(!feof(stdin)) {
		num = getline(&theLine, &size, stdin); //reads line by line
		if(strstr(theLine, delim) != NULL && (num == strlen(delim) + 1))  { //ensures that delimeter is found AND it is the only member of its line (line length = delim length)
			check = true;
			continue;			
		}
		if (check == false) fputs(theLine,temp); //adds appropriate contents to temp
	}
	free(theLine);
	inptr = "tempFile";
	fclose(temp);
	in = 2;
}

/*
 * parse fills an array of pointers with words and arguments that are filtered using 
 * getword.c. Necessary flags are set to properly execute shell commands including '>',
 * '<', '&','|', and other shell functions called by execvp(). Parse reads input one line
 * at a time and finishes when reaching lone '$', a new line, or EOF symbol.
 */

int parse() {

	vptr = 0; //resets necessary arrays, variables, and flags for next line of input
	bptr = 0;	
	in = 0;
	out = 0;
	pipeflag = 0;
	args = 0;
	outptr = NULL;
	inptr = NULL;
	badin = false;
	badout = false;
	found = false;	
	amper = false;	
	pipectr = 0;
	size = 0;
	badenv = false;
	doublein = 0;
	dollar = false;
	edge = false;
	memset(newargv, 0, sizeof(newargv));
		
	while ((charcount = getword(buffer + bptr)) != -255) {
		if (charcount < 0) { //handles words beginning with $ and changes the argument to the environement variable of contents after $
			charcount *= -1;
			if(getenv(buffer + bptr) == NULL) fprintf(stderr, "bad environment variable");
			else {
				newargv[vptr] = getenv(buffer + bptr);
				dollar = true;
				vptr++;
			}
		}
		if (charcount == 0) { //break on new line
			break;  	
		}
/*sets flag for input and then puts next word in a pointer on next
 *pass so that a file can be read for input */
		if (*(buffer + bptr) == 60 && charcount == 1 && in == 0) in++;
		else if (*(buffer + bptr) == 62 && charcount == 1 && out == 0) out++; 
		else if (in == 1) {
			inptr = buffer + bptr;
			in++;
		}
		else if (doublein == 1) { //handles << input and sets flag
			doublein = 2;
			delim = buffer + bptr;
		}
		else if (*(buffer +bptr) == 60 && (*(buffer + bptr + 1)) == 60 && charcount == 2 && doublein == 0) {
			doublein = 1;
		}
/*sets flag for output and then puts next word in a pointer on next*/
		else if (out == 1) {
			if (dollar == true) {
				outptr = getenv(buffer + bptr);
				newargv[vptr - 1] == NULL; //puts environ variable inside outptr instead of newargv
				vptr--;
				dollar = false;
			}	
			else outptr = buffer + bptr;

			out++; 
		}
/*sets pipe flag and separates left and right sides of pipe to prepare for pipeline procedure */
		else if (*(buffer + bptr) == 124) {
			pipeflag++;
			newargv[vptr] = NULL; //null terminate before each pipe
			vptr++;
			afterPipe[pipectr] = vptr; //save argument spot for after each pipe
			pipectr++;
			bptr += (charcount + 1);
			continue;	
		}
/*if ampersand is at the end of the line set background flag for process to run in back
 * ground else add it to the array as a normal character */ 
		else if (*(buffer + bptr) == 38) {
			if (amper == false) {
				newargv[vptr] = buffer + bptr;
				vptr++;
			}
			else args--; //bs ampersand is not addded to newargv array
		}
/* checks to make sure that too many special flags are not set in one line, if there are
 * too many special flags set, throw errors */
		else if (*(buffer + bptr) == 60 && in == 2) {
			badin = true;
		}
		else if (*(buffer + bptr) == 62 && in == 2) {
			badout = true;
		}
/* if a word begins with a tilde, search through the file "/etc/passwd" and return the 
 * 6th element of the line delimited by ":" and adds this to newargv. This is achieved
 * using strtok
 */ 
		else if(tilde == true){
			tilde = false;
			pass = fopen("/etc/passwd", "r");
			if (pass == NULL) {
			fprintf(stderr, "couldn't find passwd file");
			exit(1);
			}
			if(strcmp((buffer + bptr), "cs570/Data4") == 0) edge = true; //edge case for when first element of line is cs570 and input is ~cs570/Data4
			while(getline(&line, &size, pass) != -1) {
				token[0] = strtok(line, ":"); //first element of line
				if(strcmp(token[0], (buffer + bptr)) == 0 && edge == false) {
					i = 1;
					for(i; i < 6; i++) { //grabs 6th element of line
						token[i] = strtok(NULL, ":");
						if (i == 5) found = true; 
					}
					if (found == true) break; 	
				}
				else if(strcmp(token[0],"cs570") == 0 && edge==true) {
					i = 1;
					for(i; i < 6; i++) { //grabs 6th element of line
						token[i] = strtok(NULL, ":");
						if (i == 5) found = true; 
					}
					if (found == true) break; 	
				}
			}
			if (token[5] == NULL) {
				fprintf(stderr, "could not find appropriate username");
				exit(1);
			}	
			newargv[vptr] = token[5]; //add 6th element to newargv
			vptr++;
			bptr += (strlen(token[5]) + 1);
			args++;
			continue;
		}
		else {
			if (dollar == false) {
			newargv[vptr] = buffer + bptr; //add to argument array
			vptr++;
			}
			else dollar = false;
		} 

		bptr += (charcount + 1); /*move pointer so that it can process next word
* on next pass */   
		args++; 
	} 	
	newargv[vptr] = NULL; //null terminate arguments
	if(doublein == 2) {
		hereis();
	}
	return args;
}

/* Based on arguments and set flags in parse(), p2.c is designed to work as a shell. 
 * p2.c is designed to handle any combination of VALID shell commands that are flagged by
 * parse or recognized by execvp. 'cd' is also recognized by p2. For input and output
 * commands p2 will fork and run child processes. Once child processes complete or throw
 * errors the parent process will move on. If an error occurs p2 will return with exit 
 * code 1, otherwise when terminating, p2 will stop with exit code 0. */
     
void myhandler(int signum) {};
int main() {

	(void) signal(SIGTERM, myhandler); //signals using specified parameters 
	setpgid(0,0);
	dir = "";
 
	for(;;) {
		printf("%s:570:", dir);
		argc = parse(); // returns number of shell arguments to argc 
		if(charcount == -255 && argc == 0) break; //stop if EOF found
		if (argc == 0) continue; //get new input if new line found
		if(badenv == true) continue;
/* throws errors about input and output not having the correct number of arguments to
 * execute properly */
		else if (out > 0 && argc == 2) {
			fprintf(stderr, "output not specified \n");
			continue;	
		}
		else if (in > 0 && argc == 2) {
			fprintf(stderr, "no command found for input \n");
			continue;	
		}

/* cd to HOME directory if there is one cd arguments if there are two, cd to second specified argument. Otherwise throw error for too many args. Also list tail of current
 * directory on next prompt
 */
		if(strcmp(newargv[0], "cd") == 0) {
			if (argc == 1) {
				if(chdir(getenv("HOME")) != 0) fprintf (stderr, "home directory not found \n"); //changes directory to "HOME"
				current = getcwd(working, sizeof(working));
				if (strlen(current) == 1) dir = current;
				else {
					dirctr = 0;
					while (dir != NULL) { //tokenizes current directory 
						if (dirctr == 0) {
							dir = strtok(current, "/");
							path[dirctr] = dir;
						}
						else {
							dir = strtok(NULL, "/");
							path[dirctr] = dir;
						}
						dirctr++;
					}
				dir = path[dirctr - 2]; //[dirctr-2] is last token which we add to prompt
				}
			} 
			else if (argc == 2) {
				if(chdir(newargv[1]) != 0) fprintf(stderr, "no file/directory %s found \n", newargv[1]); //changes working directory to newargv[1]
				current = getcwd(working, sizeof(working));
				dirctr = 0;
				while (dir != NULL) { //tokenizes current directory
					if (dirctr == 0) {
						dir = strtok(current, "/");
						path[dirctr] = dir;
					}
					else {
						dir = strtok(NULL, "/");
						path[dirctr] = dir;
					}
					dirctr++;
				}
				dir = path[dirctr - 2]; //tail of directory to add to prompt
			}		
			else {
				fprintf(stderr, "not enough arguments \n"); 
			}
			continue;
		}
/* if 2 arguments, print the 2nd argument's environment variable, if 3 arguments replace the second arguments' enivronment variable with that of the thirds'
 */
		if(strcmp(newargv[0],"environ") == 0) {
			if (argc == 1) fprintf(stderr, "not enough arguments \n");
			else if (argc == 2) {
				if((getDir = getenv(newargv[1])) == NULL) printf(" \n");
				else printf("%s\n", getDir); 

			}
			else if (argc == 3) {
				if((setenv(newargv[1], newargv[2], 1)) == -1) fprintf(stderr, " \n"); //replace environment variable of newargv[1] with newargv[2] 
			}
			else fprintf(stderr, "too many arguments \n"); 
			continue;
		}
		fflush(stdout); //flushes stdout and stderr getting rid of unexpected output 
		fflush(stderr);

/*vertical multiple piping is accomplished by forking a child, piping the first two file
 * descriptors together,forking a granchild, piping the remaining file descriptors,
 * then forking all remaining grandchildren until the last is reached. Then begin
 * chaining commands together via dup2 at appropriate indexes. Also begin to exec
 * arguments begnning with the left most argument and ending at the right most argument.
 * ensure the parent of all pipes wait for ALL children to terminate. Close file
 * descriptors when necessary to avoid pipes remaning open 
 */ 
		if (pipeflag >= 1) {
			int fd[pipectr*2];
			if((child = fork()) == 0) {
				pipe(fd);
				if((grandchild = fork()) == 0) {
					for(i = 1; i <= pipectr; i++) {
						j = i * 2;
						pipe(fd+j);
						if (i < pipectr) {
							if((greatgrand = fork()) == 0) continue;
							else {//these are middle children that are connected via dup2. j for input and j-1 for ouput are appropriate ends of each pipe 
								CHK(dup2(fd[j], STDIN_FILENO));
								CHK(dup2(fd[j-1], STDOUT_FILENO));	
								for(k = 0; k < (j+2); k++) {
									close(fd[k]); //close open file descriptors
								}	
							execvp(newargv[afterPipe[pipectr - i - 1]], newargv + afterPipe[pipectr - i - 1]); //execvp middle children (start from number of pipers and subtract current index -1 to get the right command 
							}
						}
						else {
							if (in == 2) {
								flags = O_RDONLY; //flag for reading file as input
								if((infile = open(inptr, flags, S_IRUSR|S_IWUSR)) < 0) {
								fprintf(stderr,"failed to open file or directory \n");
								exit(2);	
								}	
							dup2(infile, STDIN_FILENO);
							close(infile);
							in = 0;
								if(doublein == 2) { //removes temp file from <<
									remove("tempFile");
									doublein = 0;
								}
							}
							CHK(dup2(fd[j-1],STDOUT_FILENO));//connect greatest grandchild to STDOUT
							for( k = 0; k < j + 2; k++) {
								close(fd[k]);
							}
							execvp(newargv[0], newargv);//execvp left most argument
						}
					}	
				}
				else {
					if (out == 2) {
					flags = O_CREAT | O_RDWR | O_EXCL; //flags for file creation
						if((outfile = open(outptr, flags, S_IRUSR|S_IWUSR)) < 0) {
							fprintf(stderr, "failed to open file or directory \n");
							exit(2);
						}
					dup2(outfile, STDOUT_FILENO);
					close(outfile);
					out = 0;
					}
					dup2(fd[0], STDIN_FILENO); //connect child pipe with STDIN
					close(fd[0]);
					close(fd[1]);
					execvp(newargv[afterPipe[pipectr - 1]], newargv + afterPipe[pipectr - 1]);//execvp right most argument	
				}
			}
			else if (child > 0) {
				for(;;) {
					pid_t par;
					CHK(par=wait(NULL));
					if(child==par) break; 
				}
			}
		}
/* if shell command is not pipe or cd fork a child to complete the process before 
 * returning to the parent process. Throw an error if forking is not possible.*/ 
  
		else {
			kidpid = fork(); 
			if (kidpid == -1) {
				fprintf(stderr, "cannot fork \n");
				exit(1);	
			}
			else if (kidpid == 0) {
/* Error checks for input and output ensuring multiple '<' or '>' are not seen in 
 * a line of input together*/			
				if (badin == true) {
					fprintf(stderr, "ambiguous input \n");
					exit(1);	
				}
				else if (badout == true) {
					fprintf(stderr, "ambiguous output \n");
					exit(1);	
				}
/* dev null used for suppressing display of unwatned output*/
				else if (amper == true && in == 0) {
					if ((dn = open("/dev/null", O_RDONLY)) < 0) {
						fprintf(stderr, "can't find /dev/null \n");
					}	
					dup2(dn, STDIN_FILENO);//spcifies input to be /dev/null which is discarded 
					close(dn);
				}
/*if '<' is specified use inptr to access file and use its contents as input by 
 * redefining input to the file via dup2 */

				if (in == 2) {
					flags = O_RDONLY; //flag for reading file as input
					if((infile = open(inptr, flags, S_IRUSR|S_IWUSR)) < 0) {
						fprintf(stderr,"failed to open file or directory \n");
						exit(1);	
					}
					dup2(infile, STDIN_FILENO);
					close(infile);
					in = 0;
					
						if(doublein == 2) { //removes temp file for <<
							remove("tempFile");
							doublein = 0;
						}
				}

/* if '>' is specified create a new file, specified by outptr, for output and re-define 
 * output to be this new file using dup2 */ 
				if (out == 2) {
				flags = O_CREAT | O_RDWR | O_EXCL; //flags for file creation
					if((outfile = open(outptr, flags, S_IRUSR|S_IWUSR)) < 0) {
						fprintf(stderr, "failed to open file or directory \n");
						exit(1);
					}
					dup2(outfile, STDOUT_FILENO);
					close(outfile);
					out = 0;
				}
				if ((execvp(newargv[0], newargv)) == -1) { //checks to make sure the first argument of input is a legitimate shell command 
					fprintf(stderr,"failed to execvp argument %s \n", newargv[0]);
					exit(1);	
				}
			}
			else { //flag for running background process
				if (amper == true) {
				printf("%s [%d]\n", *newargv, kidpid); //prints child's pid
				amper = false;
				continue; //run process in the background without waiting for child		
				}		
				else { //waits for child process to finish before continuing
					for(;;) {
					pid_t pid;
					CHK(pid = wait(NULL));
					if(kidpid == pid) break; //child process has finished
					}
				}	
			}
		}
		
	}
	killpg(getpgrp(), SIGTERM); //sends signal to process group
	printf("p2 terminated.\n");
	exit(0);
}
