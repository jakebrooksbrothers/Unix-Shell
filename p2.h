#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>
#include "getword.h"
#include "CHK.h"
#define MAXITEM 100 /* max number of words per line */
#define MAXSTORAGE 25500

extern bool amper;
extern bool tilde; 
//used to determine if & is at the end of a line or not (used in parse() and getword.c)

int parse();


