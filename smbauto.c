#include<stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * String parsing section.
 */

int count(char c, char* str){
	int count = 0;
	while(*str) 
		if (*str++ == c) 
			++count;
	return count;
}

char* substring(char *str, int a, int b){
	int l = strlen(str);
	if (l<=0 || a > l || b > l || a < 0){
		return NULL;
	}
	
	char *res = (char*) malloc(sizeof(char) * (l + 1) );
	return strncpy(res, str+a, b-a);
}

char* formatSpaces(char *str, char *new){
	/*
	 * In fstab spaces in username, share etc. need to be represented by "%20".
	 */
	int c = count(' ', str);
	int string_size = strlen(str);
	char *res = (char*) malloc(sizeof(char) * (string_size + 1 + 3 * c) );

	int pos = 0;
	for(int i = 0; i < string_size; ++i){
		if(str[i] == ' '){
			strcat(res, substring(str, pos, i));
			strcat(res, new);
			pos = i + 1;
		}
	}
	strcat(res, substring(str, pos, string_size));
	
	return res;
}

int indexOf(char c, char *str, int start){
	char * t;
	int res = -1;
	int l = strlen(str);
	if (start >= 0){
		for (int i = start; i < l; ++i){
			if (str[i]==c){
				res = i;
				break;
			}
		}
	}
	return res;
}

char* getLine(char* str, int n){
	int a = 0;
	int b = 0;
	char* res;
	
	if(n < 0 || count('\n', str) < n){
		return NULL;
	}
	
	if(n > 0){
		for (int i = 0; i < n; ++i){
			a = indexOf('\n', str, a)+1;
		}
	}
	
	if (indexOf('\n', str, a)!=-1){
		b = indexOf('\n', str, a);
	} else {
		b = strlen(str);
	}
	
	res = substring(str, a, b);
	
	return res;
}

char* parseMountpoint(char* line){
	char* res;
	
	res = substring(line, indexOf(' ', line, 0) + 1, indexOf(' ', line, indexOf(' ', line, 0) + 1));
	
	return res;
}

/*
 * Some filehandling.
 */

char* read(char* filename){
    char *buffer = NULL;
    int string_size, read_size;
	
    FILE *handler = fopen(filename, "r");
	
	if (handler){
		fseek(handler, 0, SEEK_END);
		string_size = ftell(handler);
		rewind(handler);
		
		buffer = (char*) malloc(sizeof(char) * (string_size + 1) );
		read_size = fread(buffer, sizeof(char), string_size, handler);
		buffer[string_size] = '\0';
		
		if (string_size != read_size){
			free(buffer);
			buffer = NULL;
		}
		
		fclose(handler);
	}
	return buffer;
}

void write(char* text){
	FILE * stream;
	stream = fopen("/etc/fstab", "w+");
	fprintf(stream, "%s", text);
	fclose(stream);
}

/*
 * Actual operations
 */

void add(char* server, char* share, char* user, char* pass, char* mountpoint){
	/*
	 * <addr>:/<share> <mntpoint> url soft,automounted,_netdev,url==smb://<usr>:<pw>@<addr>/<share> 0 0
	 * 
	 * Another way to mount would be:
	 * /System/Volumes/Data/<dir> -fstype=smbfs ://usr:pw@<addr>:/<share>
	 * 
	 * replace spaces in creds or addr (" ") by %20 (\040 in Linux)
	 */
	
	char *userForm = formatSpaces(user, "%20");
	char *passForm = formatSpaces(pass, "%20");
	char *shareForm = formatSpaces(share, "%20");
	char *mountpointForm = formatSpaces(mountpoint, "_");
	
	char url[324];
	strcpy(url, "smb://");
	strcat(url, userForm);
	strcat(url, ":");
	strcat(url, passForm);
	strcat(url, "@");
	strcat(url, server);
	strcat(url, "/");
	strcat(url, shareForm);
	
	char entry[775];
	strcpy(entry, server);
	strcat(entry, ":/");
	strcat(entry, shareForm);
	strcat(entry, " ");
	strcat(entry, mountpointForm);
	strcat(entry, " url soft,automounted,_netdev,url==");
	strcat(entry, url);
	strcat(entry, " 0 0");
	
	char *text = read("/etc/fstab");
	
	if(getLine(text, 0) != NULL)
		strcat(text, "\n");
	
	strcat(text, entry);
	
	write(text);
	
	system("automount -cv");
	
	free(userForm);
	free(passForm);
	free(shareForm);
}

void rem(int n){
	char* buffer = read("/etc/fstab");
	char* line;
	int c = count('\n', buffer);
	int string_size = strlen(buffer);
	char* text = (char*) malloc(sizeof(char) * (string_size + 1 - strlen(getLine(buffer, n-1))) );
	
	
	for(int i = 0; i <= c; ++i){
		if (i != n-1){
			strcat(text, getLine(buffer, i));
			strcat(text, "\n");
		} else {
			line = getLine(buffer, i);
		}
	}
	
	if (strlen(text) > 0)
		text = substring(text,0,strlen(text)-1);
	
	char *mountpoint = parseMountpoint(line);
	
	write(text);
	
	char cmd1[100];
	strcpy(cmd1, "diskutil unmount ");
	strcat(cmd1, mountpoint);
	system(cmd1);
	
	char cmd2[100];
	strcpy(cmd2, "rm -r ");
	strcat(cmd2, mountpoint);
	system(cmd2);
	
	free(buffer);
	free(text);
}

void list(void){
	char* buffer = read("/etc/fstab");
	char* line;
	int c = count('\n', buffer) + 1;
	
	for(int i = 0; i < c; ++i){
		printf("%d) %s\n", i+1 , getLine(buffer, i));
	}
	free(buffer);
}

/*
 * Handling user input.
 */

void myinput(char *question, char *answer){
	printf("%s ",question);
	scanf("%[^\n]s",answer);
	fflush(stdin);
}

int main(){
	char option[1];
	char n[2];
	
	system("touch /etc/fstab");
	
	while(1) {
		printf("\n1) add smb\n");
		printf("2) remove smb\n");
		printf("3) show smb's\n\n");
		
		myinput("Choose one of the options above: ", option);
		
		if (strcmp(option, "1") == 0) {
			char server[15];
			char share[100];
			char user[100];
			char pass[100];
			char mountpoint[300];
	
			char keep[3] = "no";
			
			myinput("Server (ip): ", server);
			myinput("Share (dir): ", share);
			myinput("Username: ", user);
			myinput("Password: ", pass);
			myinput("Mountpoint (dir): ", mountpoint);
			
			myinput("\nContinue? (y/n): ", keep);
			
			if(keep[0]=='n'){
				continue;
			}
			
			add(server, share, user, pass, mountpoint);
			
		} else if (strcmp(option, "2") == 0) {
			list();
			myinput("\nWhich one should be removed? ", n);
			rem(atoi(n));
		} else if (strcmp(option, "3") == 0) {
			list();
		} else {
			printf("%s is not an option.\n", option);
		}
		fflush(stdin);
	}
	
	return 0;
}