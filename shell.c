#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
void pipe_cmd(char **);
void normal_cmd(char *);
char** command_parser(char *, char *);
int SIZE = 10;

void ctrlC(int sig){
	if(sig == 2 || sig == 20){
	}
	else{
		kill(0,SIGCONT);
	}
}

char **command_parser(char *string, char *delim){
	
	char **arg = (char **)malloc(sizeof(char *) * SIZE);
	int i = 1, ind = 0;
	char *p = strtok(string, delim);
	while(p){
		if(ind >= i * SIZE){
			arg = realloc(arg, sizeof(char *) * ( SIZE * (++i)));
		}
		//printf("%s",p);
		arg[ind] = p;
		ind++;
		p = strtok(NULL, delim);
	}
	arg[ind] = NULL;
	return arg;
}
char **redirection_parser(char *string, char *delim){
	char **files = (char **)malloc(sizeof(char *) * SIZE);
	char **arg;
	int i = 1, ind = 0, loop_count = 0;
	char *p = strtok(string, delim);
	while(p){
		if(ind >= i * SIZE){
			arg = realloc(arg, sizeof(char *) * ( SIZE * (++i)));
		}
		if(loop_count != 0){
			
			arg = command_parser(p, " ");
			//printf("%s", arg[0]);
			files[ind] = arg[0];
			ind++;
		}
		loop_count += 1;
		p = strtok(NULL, delim);
	}
	files[ind] = NULL;
	return files;
}
char **arg_parser(char *string, char *delim){
	int count = 2;
	char **arg = (char **)malloc(sizeof(char *) * SIZE);
	int i = 1, ind = 0;
	char *p = strtok(string, delim);
	while(p){
		if(ind >= i * SIZE){
			arg = realloc(arg, sizeof(char *) * ( SIZE * (++i)));
		}
		
		if((strcmp(p, "<") == 0 )|| (strcmp(p, ">") == 0 )){
			count = 0;
		}
		else if(count != 1){
			arg[ind] = p;
			ind++;
		}
		count++;
		p = strtok(NULL, delim);
	}
	arg[ind] = NULL;
	return arg;
}
void pipe_cmd(char **arg){
	int k, pfd[2], pid, tempfd, fd, l, i, index;
	char **args, *redirection, **file_name, **arguments;
	char *op_redirection, **op_files, *ip_redirection, **ip_files, *background;
	for(k = 0; arg[k]; k++){
		char arg_copy[strlen(arg[k])];
		strcpy(arg_copy, arg[k]);
		op_redirection = strchr(arg[k], '>');
		ip_redirection = strchr(arg[k], '<');
		if(arg[k][strlen(arg[k]) - 1] == '&'){
			background = strchr(arg[k], '&');
			arg[k][strlen(arg[k]) - 1] = '\0';
		}
		if(op_redirection){
			op_files = redirection_parser(arg[k], ">");
			for(i = 0; op_files[i]; i++){
				fd = open(op_files[i],  O_CREAT | O_WRONLY  , S_IRUSR | S_IWUSR );
				close(fd);
			}
		}
		strcpy(arg[k], arg_copy);
		if(ip_redirection){
			ip_files = redirection_parser(arg[k], "<");
			for(l = 0; ip_files[l]; l++){

			}
		}
		strcpy(arg[k], arg_copy);
		args = arg_parser(arg[k], " ");
		pipe(pfd);
		pid = fork();
		if(pid == 0) {
			
			if(k != 0)
				dup2(tempfd, STDIN_FILENO);
			if(arg[k+1] != NULL)
				dup2(pfd[1], STDOUT_FILENO);
			if(op_redirection){
				close(STDOUT_FILENO);
				fd = open(op_files[i - 1],  O_CREAT | O_WRONLY  , S_IRUSR | S_IWUSR );
			}
			if(ip_redirection){
				close(STDIN_FILENO);
				fd = open(ip_files[l - 1], O_RDONLY);
			}
			close(pfd[0]);
			if(execvp(args[0], args ) == -1)
			perror("");

		}
		else {
			close(pfd[1]);
			if(!background){
				wait(0);
			}
			tempfd = pfd[0];
		}
	}
}

void normal_cmd(char* arguments){
	int pid, pfd[2], fd, i, k, error;
	char **args, **arg, *op_redirection, **op_files, *ip_redirection, **ip_files, *background = NULL;
	op_redirection = strchr(arguments, '>');
	ip_redirection = strchr(arguments, '<');
	if(arguments[strlen(arguments) - 1] == '&'){
		background = strchr(arguments, '&');
		arguments[strlen(arguments) - 1] = '\0';
	}
	char arg_copy[strlen(arguments)];
	strcpy(arg_copy, arguments);
	if(op_redirection){
		op_files = redirection_parser(arguments, ">");
		for(i = 0; op_files[i]; i++){
			fd = open(op_files[i],  O_CREAT | O_WRONLY  , S_IRUSR | S_IWUSR );
			close(fd);
		}
	}
	strcpy(arguments, arg_copy);

	if(ip_redirection){
		ip_files = redirection_parser(arguments, "<");
		for(k = 0; ip_files[k]; k++){
			
		}
	}	
	strcpy(arguments, arg_copy);
	args = arg_parser(arguments, " ");
	pid = fork();
	if(pid == 0) {
		if(op_redirection){
			close(STDOUT_FILENO);
			fd = open(op_files[i - 1],  O_CREAT | O_WRONLY  , S_IRUSR | S_IWUSR );
		}
		if(ip_redirection){
			close(STDIN_FILENO);
			fd = open(ip_files[k - 1], O_RDONLY);
		}
		if(execvp(args[0], args ) == -1)
			perror("");

	}
	else {
		if(!background){
			wait(0);
		}
	}
}
int main() {
	int i, size;
	char c;
	char *cmd, **arguments, **arg;
	struct sigaction sa;
	sa.sa_handler = ctrlC;
	sa.sa_flags = 0;
	signal(SIGINT, ctrlC);
	signal(SIGTSTP, ctrlC);
	while(1) {
		printf("prompt>");
		cmd = (char *)malloc(sizeof(char) * 128);
		i = 0;
		size = 100;
		c = getchar();
		if(c == EOF){
			return 0;
		}
		else{
			while( c != '\n'){
				if(i >= size){
					size *= 2;
					cmd = (char *)realloc(cmd, sizeof(char) * size);
				}
				cmd[i++] = c;
				c = getchar();
				if(c == EOF)
					continue;
			}	
			arguments = command_parser(cmd, ";");
			for(i = 0; arguments[i]; i++){	
				char *p = strchr(arguments[i], '|');	
				if(p){
					arg = command_parser(arguments[i], "|");
					pipe_cmd(arg);
				}
				else{
					normal_cmd(arguments[i]);
				}
			}
		}
	}		
	
	return 0;
}
