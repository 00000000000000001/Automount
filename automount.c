#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* substring(char *str, int a, int b){
	int l = strlen(str);
	if (l<=0 || a > l || b > l || a < 0){
		return NULL;
	}
	
	char *res = (char*) malloc(sizeof(char) * (l + 1) );
	return strncpy(res, str+a, b-a);
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

int count(char c, char* str){
	int count = 0;
	while(*str) 
		if (*str++ == c) 
			++count;
	return count;
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

///

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

void mkdir(char* mountpoint){
	FILE *fp;
	char path[1035];
	
	char cmd[300];
	strcpy(cmd, "/bin/mkdir -p ");
	strcat(cmd, mountpoint);

	fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}

	while (fgets(path, sizeof(path), fp) != NULL) {
		printf("%s", path);
	}

	pclose(fp);
}

// add smb
void o1(char* server, char* share, char* user, char* pass, char* mountpoint){
	char url[324];
	strcpy(url, "smb://");
	strcat(url, user);
	strcat(url, ":");
	strcat(url, pass);
	strcat(url, "@");
	strcat(url, server);
	strcat(url, "/");
	strcat(url, share);
	
	char entry[775];
	strcpy(entry, server);
	strcat(entry, ":/");
	strcat(entry, share);
	strcat(entry, " ");
	strcat(entry, mountpoint);
	strcat(entry, " url nofail,x-systemd.device-timeout=1ms,automounted,_netdev,url==");
	strcat(entry, url);
	strcat(entry, " 0 0");
	
	char *text = read("/etc/fstab");
	
	if(getLine(text, 0) != NULL)
		strcat(text, "\n");
	
	strcat(text, entry);
	
	mkdir(mountpoint);
	write(text);
	system("automount -cv");
}

///

char* parseMountpoint(char* line){
	char* res;
	
	res = substring(line, indexOf(' ', line, 0) + 1, indexOf(' ', line, indexOf(' ', line, 0) + 1));
	
	return res;
}

// remove smb
void o2(int n){
	char* buffer = read("/etc/fstab");
	char* line;
	int c = count('\n', buffer) + 1;
	int string_size = strlen(buffer);
	char* text = (char*) malloc(sizeof(char) * (string_size + 1 - strlen(getLine(buffer, n-1))) );
	
	
	for(int i = 0; i < c; ++i){
		if (i != n-1){
			strcat(text, getLine(buffer, i));
		} else {
			line = getLine(buffer, i);
		}
	}
	
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

// show smb's
void o3(void){
	char* buffer = read("/etc/fstab");
	char* line;
	int c = count('\n', buffer) + 1;
	
	for(int i = 0; i < c; ++i){
		printf("%d) %s\n", i+1 , getLine(buffer, i));
	}
	free(buffer);
}

int main(){
	int option;
	int n;
	
	system("touch /etc/fstab");
	
	while(1) {
		printf("\n1) add smb\n");
		printf("2) remove smb\n");
		printf("3) show smb's\n\n");
		
		printf("Choose one of the options above: ");
		scanf("%d", &option);  
		
		if (option == 1) {
			char server[15];
			char share[100];
			char user[100];
			char pass[100];
			char mountpoint[300];
	
			char keep[3] = "no";
	
			printf("\nServer (ip): ");
			scanf("%14s", &server[0]);
			printf("Share (dir): ");
			scanf("%99s", &share[0]);
			printf("Username: ");
			scanf("%99s", &user[0]);
			printf("Password: ");
			scanf("%99s", &pass[0]);
			printf("Mountpoint (dir): ");
			scanf("%299s", &mountpoint[0]);
	
			printf("\nContinue? (y/n): ");
			scanf("%2s", &keep[0]);
			
			if(keep[0]=='n'){
				continue;
			}
			
			o1(server, share, user, pass, mountpoint);
			
		} else if (option == 2) {
			o3();
			printf("\nWhich one should be removed? ");
			scanf("%d", &n);
			o2(n);
		} else if (option == 3) {
			o3();
		} else {
			printf("%d is not an option.\n", option);
		}
		fflush(stdin);
	}
	
	return 0;
}