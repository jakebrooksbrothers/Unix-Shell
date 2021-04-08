//Jacob Brooks
//Professor Carroll
//CS 570
//Due 9/15/20

/*
 *getword.c takes input from stdin and delimits this input into "words" that are defined 
 *the newly delimited word in character array w. Typical word delimeters are spaces,  
 *tabs, and newline characters. Metacharacters that count as their own word include '<',
 *'>', '&', '<<', and '|'. Finally, metacharacters '\', '$', and '~' follow special rules
 *that could alter delimitation, word length, and the word itself.
 */

#include "getword.h"
#include <stdlib.h>
#include <stdbool.h>
#include "p2.h"

bool amper;
bool tilde;
int getword(char *w) { 

	int iochar;	//declarations
	int check;
	int n = 0;
	int size = 0;
	bool is$ = false;
	memset(w, 0, sizeof w);

	while ((iochar = getchar()) != EOF) {
		if (iochar == 10 && n == 0) break; //ensures no current word is being collected (n = 0),to return newline with length of zero
		else if (iochar == 10 && n!= 0) { //stops collecting current word (n!=0) and ungets newline char to return it to stdin on next call of getword.c ensuring that it can be re-evaluated for special instructions regarding its type. 	 
			ungetc(iochar, stdin);	 
			break;
		}
		else if (iochar == 92) { //checks all special cases following a '\' 
			check = getchar(); //check represents next character to be read from stdin 
			if (check == 10) { //treats '\newline' as a space and either skips both chars if there is no current word OR breaks loop to terminate collecting current word 
				if (n == 0) continue; 
				else break;
			}
			else { //treats metacharacters '$', '&', '<', '>', '\', '|', and '~' as normal characters by not collecting '\' and collecting the next character as normal by NOT delimiting after collection.
				w[size] = check;
				size++;
				
			} 
		}
		else if (iochar == 38) { //collects '&' by first terminating current word if it exists by breaking, then ungetting '&' to recollect it from stdin on next function call allowing it to be read again for special instructions. Breaks to terminate the word.
			if (n == 0) {
				w[0] = iochar;
				size = 1;
				n = 1;
				amper = true;
				break;
			}
			else {
				ungetc(iochar, stdin);
				break;	
			}
		}
		else if (iochar == 62) { //collects '>' by first terminating current word if it exists by breaking, then ungetting '>' to recollect it from stdin on next function call allowing it to be read again for special instructions. Breaks to terminate the word. 
			if (n == 0) {
				w[0] = iochar;
				size = 1;
				n = 1;
				break;
			}
			else {
				ungetc(iochar, stdin);
				break;
			}	
		}
		else if (iochar == 124) { //collects '|' by first terminating current word if it exists by breaking, then ungetting '|' returning it to stdin on next function call so that it can be re-evaluated for special instructions. Breaks to terminate the word. 
			if (n == 0) {
				w[0] = iochar; 
				size = 1;
				n = 1;
				break;
			}
			else {
				ungetc(iochar, stdin);
				break;	
			}
		}
		else if (iochar == 60) { //first terminates current word if it exists by breaking, then gets next char from stdin to determine if '<' or'<<'. if '<<' is found collect it as a word and terminate. If only '<' is found unget char after '<' to ensure it is properly read from stdin on next function call and collect and terminate single '<'.  
			if (n == 0) {
				if((check = getchar()) == 60) {
					w[0] = iochar;
					w[1] = check;
					size = 2;
					n = 2;
					break;
				}
				else {
					w[size] = iochar;
					size++;
					n = 1;
					ungetc(check, stdin);
					break;
				}
			}
			else {
				ungetc(iochar, stdin);
				break;	
			}
		}
		else if ((iochar == 32 || iochar == 9) && n == 0) continue; //if a space or tab is detected with no current word, continue to not collect until a valid character is found 
		else if ((iochar == 32 || iochar == 9) && n!= 0) {
				break; // if a space or tab is detected with a current word, break to finish collecting the current word
		}
		else if (iochar == 36 && n == 0) is$ = true; //if '$' is found at the beginning of a word mark it for special rules regarding word length and output
		else if (iochar == 126 && n == 0) { // if '~' is found at the beginning of a word collect home directory and copy it into w. Update length and size appropriately.Continue in order to append the rest of the word after '~' to the home directory so that both the path and valid chars after '~' are collected together. Also set an external flag
			tilde = true; 
			continue;
		} 
		else { //assume that iochar is a valid non delimiting character that can be collected as part of the current word 
			w[size] = iochar;	
			size++;
		} 
			n++; //assume a character was added to word so increment length 
		if (iochar != 32 && iochar != 10 && iochar != 36 && iochar != 255 && iochar != 92) amper = false; //checks wheter or not & is the last character in the line and gets passed as a special character if it is.
		if (size == 254) break; //limit size of collected word to 254 characters + '\0' to null terminate the string
	}
	 w[size] = '\0'; //null terminates to ensure no unexpected overflow occurs 
	 if (iochar == EOF && n == 0) return -255; //ensures all words are finished being collected before returning -255 
	 else {
		if (is$ == true) { //decrement n to account for extra length added by counting the '$' when it was not collected. Return -n due to word beginning with '$'
			is$ = false; 
			n--; 
			return n*= -1;
		}
		else return n; //returns delimited word's length  
	}
}
