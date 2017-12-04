#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <iostream> 
#include <sys/wait.h>

#define MAX_LEN 550
#define ALLOWED_LEN 512

int getNoOfWords(char* str);
void copySubstring(char* src, char* dest, int s, int e);
char* getWord(char* src, int s, int e);
char** splitString(char* str, int& num_words, bool &isBackground);
bool isBlank(char c);


using namespace std;
int main() {
	char line[MAX_LEN];
	
	while(1){
		printf("shell> ");
		fgets(line, MAX_LEN, stdin);
		
		int num_words = 0;
		bool is_background = false;
		char** command = splitString(line, num_words, is_background);
		
		if(num_words == 0) {
			continue;
		}
		if(!strcmp(command[0], "exit")) {
			kill(0, SIGKILL);
		}
		pid_t pid = fork();
		
		if(pid > 0) { // parent
			if(!is_background){
				int status;
				waitpid(pid, &status, 0);
			}
		} else if(pid == 0) { // child
			// cd is handled explicitly
			int r = 0;
			if(!strcmp(command[0], "cd")) {
				r = chdir(command[1]==NULL? "" : command[1]);
			} else {
				r = execvp(command[0], command);
			}
			if(r < 0) {
				perror ("Error");
			}
			
		} else { // error
			
		}
		
		for(int i = 0; i <= num_words; i++) {
			delete[] command[i];
		}
		delete[] command;
	}
	return 0;
}

bool isBlank(char c) {
	return c == ' ' || c == '\t' || c =='\n';
}

int getNoOfWords(char* str) {
	int len = strlen(str);
	if(!len) return 0;
	
	int words_num = 0;
	if(!isBlank(str[0]))
		words_num = 1;
	
	for(int i = 0; i < len; i++) {
		if(isBlank(str[i]) && i+1 != len && !isBlank(str[i+1]))
			words_num++;
	}
	return words_num;
}

void copySubstring(char* src, char* dest, int s, int e) {
	int c = 0;
	while(s < e) {
		dest[c++] = src[s++];
	}
	dest[c] = '\0';
}

char* getWord(char* src, int s, int e) {
	char* word = new char[e - s + 1];
	copySubstring(src, word, s, e);
	return word;
}

char** splitString(char* str, int& num_words, bool& isBackground) {
	int len = strlen(str);
	num_words = getNoOfWords(str); // to know how much to allocate
	char** ret = new char*[num_words + 1];
	
	int i = 0, word_ind = 0, stt = 0;
	while(i < len) {
		if(!isBlank(str[i])) {
			i++;
			continue;
		}
		if(i != 0){
			char* word = getWord(str, stt, i);
			int wordlen = strlen(word);
			if(word_ind == num_words - 1 && word[wordlen - 1] == '&') {
				isBackground = true;
				if(wordlen == 1) {
					num_words --;
				} else {
					word[wordlen - 1] = '\0';
					ret[word_ind++] = word;
				}
			} else {
				ret[word_ind++] = word;
			}
		}
		while(i < len && isBlank(str[i]))i++;
		stt = i;
	}
	if(stt < len) {
		char* word = getWord(str, stt, i);
		int wordlen = strlen(word);
		if(word[wordlen - 1] == '&') {
			isBackground = true;
			if(wordlen == 1) {
				num_words --;
			} else {
				word[wordlen - 1] = '\0';
				ret[word_ind++] = word;
			}
		} else {
			ret[word_ind++] = word;
		}
	}
	
	ret[num_words] = NULL;
	
	return ret;
}
