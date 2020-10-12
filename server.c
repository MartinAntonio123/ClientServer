#include "header.h"
#include <string.h>
int semid;
long len_dir(char* filename){
	DIR* dir;
	struct dirent* direntry;
	struct stat mystat;
	long lengthb = 0;
	dir = opendir(filename);
	while ( (direntry = readdir(dir)) != NULL ) {
		stat(direntry->d_name, &mystat);
		if (!S_ISDIR(mystat.st_mode)){
			lengthb = lengthb + strlen(direntry->d_name) + 1;
		}
	}
	closedir(dir);
	return lengthb;
}
void write_log(int code, char* data, char* root){
	mutex_wait(semid, MUTEXWRITE);
	FILE * file;
	char filename[PATH_MAX + NAME_MAX + 1];
	sprintf(filename, "%s/%s", root, "log.txt");
	file = fopen(filename,"a");
	time_t t;
  struct tm *tm;
  char fechayhora[100];
  t = time(NULL);
  tm = localtime(&t);
  strftime(fechayhora, 100, "%d/%m/%Y", tm);
	fprintf(file, "date: %s code: %i data: %s\n",fechayhora, code, data);
	fclose(file);
	mutex_signal(semid, MUTEXWRITE);
}
void write_client(int nsfd, int code, long length, char* data){
	write(nsfd, &code, sizeof(code));
	write(nsfd, &length, sizeof(length));
	write(nsfd, data, length * sizeof(char));
	printf("Sent: %i codigo: %i lenght: %li text: %s\n", getpid(), code, length, data);
}
void serves_101(char* arch, long length, long data_read, char* root, int nsfd){
	if (data_read != length){
		write_client(nsfd, 203, 14, "Internal error");
		write_log(203, "Internal error", root);
    exit(0);
	}
	struct stat info;
	char filename[PATH_MAX + NAME_MAX + 1];
	sprintf(filename, "%s%s", root, arch);
	lstat(filename, &info);
  if((strstr(filename, "..") != NULL) || (arch[0] != '/') ){
		write_client(nsfd, 203, strlen("Internal error"), "Internal error");
		write_log(203, "Internal error", root);
  }
	else if(access(filename, F_OK) == -1){
		write_client(nsfd, 202, strlen("File not found"), "File not found");
		write_log(202, "File not found", root);
  }
  else if(access(filename, R_OK|W_OK) == -1){
		write_client(nsfd, 201, strlen("Permision denied"), "Permision denied");
		write_log(201, "Permision denied", root);
  }
	else if (S_ISDIR(info.st_mode)) {
		write_client(nsfd, 205, strlen("Path is a directory"), "Path is a directory");
		write_log(205, "Path is a directory", root);
	}
	else{
		int fd_in;
		fd_in = open(filename, O_RDONLY);
		long lengthb;
		lengthb = lseek(fd_in, 0, SEEK_END);
		lseek(fd_in, 0, SEEK_SET);
		char *buffer;
		buffer = (char*) malloc(lengthb * sizeof(char));
		long nbytes;
		while ( (nbytes = read(fd_in, buffer, lengthb)) != 0 ) {
			if (nbytes == lengthb) {
				write_client(nsfd, 301, lengthb/sizeof(char), buffer);
				free(buffer);
				close(fd_in);
				write_log(301, filename, root);
			}
			else{
				free(buffer);
				close(fd_in);
				write_client(nsfd, 203, strlen("Internal error"), "Internal error");
				write_log(203, "Internal error", root);
				printf("\n\nerror interno\n\n" );
			}
		}
		/*/
		write_client(nsfd, 301, strlen("funciona101"), "funciona101");
		write_log(301, filename, root);*/
	}
}
void serves_102(char* arch, long length, long data_read, char* root, int nsfd){
	if (data_read != length){
		write_client(nsfd, 203, strlen("internal error"), "internal error");
		write_log(203, "internal error", root);
	}
	struct stat info;
	char filename[PATH_MAX + NAME_MAX + 1];
	sprintf(filename, "%s%s", root, arch);
	lstat(filename, &info);
  if((strstr(filename, "..") != NULL) || (arch[0] != '/') ){
		write_client(nsfd, 203, strlen("Internal error"), "Internal error");
		write_log(203, "Internal error", root);
  }
	else if(access(filename, F_OK) == -1){
		write_client(nsfd, 206, strlen("Dir not found"), "Dir not found");
		write_log(206, "Dir not found", root);
  }
  else if(access(filename, R_OK|W_OK) == -1){
		write_client(nsfd, 201, strlen("Permision denied"), "Permision denied");
		write_log(201, "Permision denied", root);
  }
	else if (!S_ISDIR(info.st_mode)) {
		write_client(nsfd, 207, strlen("Path is a not directory"), "Path is not a directory");
		write_log(207, "Path is not a directory", root);
	}
	else{
		DIR* dir;
		struct dirent* direntry;
		struct stat mystat;
		long lengthb = len_dir(filename);
		char* data;
		data = (char*) malloc(lengthb * sizeof(char));
		int i = 1;
		dir = opendir(filename);
		while ( (direntry = readdir(dir)) != NULL ) {
			stat(direntry->d_name, &mystat);
			if (!S_ISDIR(mystat.st_mode)){
				if(i){
					sprintf(data, "%s",direntry->d_name);
					i = 0;
				}
				else{
					sprintf(data, "%s %s", data, direntry->d_name);
				}
				printf(" %s ", direntry->d_name );
			}
		}
    closedir(dir);
		printf("\n");
		write_client(nsfd, 302, lengthb, data);
		write_log(302, filename, root);
		free(data);
	}
}
void serves_client(int nsfd, char* root) {
	int code;
	long length, data_read;
	write_client(nsfd, 1, strlen("hola"), "hola");
	write_log(1, "new connection", root);
	do{
		char* data;
		read(nsfd, &code, sizeof(code));
		read(nsfd, &length, sizeof(length));
		data = (char*) malloc(length * sizeof(char));
		data_read = read(nsfd, data, length * sizeof(char));
		data[length] = '\0';
		printf("Pid: %i codigo: %i lenght: %li text: %s\n", getpid(), code, length, data);

		if (code == 101) {
			serves_101(data, length, data_read, root, nsfd);
		}
		else if(code == 102){
			serves_102(data, length, data_read, root, nsfd);
		}
		else if(code == 103){
			write_log(103, "Adios", root);
		}
		else{
			write_client(nsfd, 204, 5, "Error");
			write_log(204, "Error", root);
		}
		free(data);
	}while (code != 103);
	close(nsfd);
	printf("%i conexion terminada\n", getpid());
}

void server(char* ip, int port, char* program, char* root) {
	int sfd, nsfd, pid;
	struct sockaddr_in server_info, client_info;

	if ( (sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror(program);
		exit(-1);
	}

	server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = inet_addr(ip);
	server_info.sin_port = htons(port);
	if ( bind(sfd, (struct sockaddr *) &server_info, sizeof(server_info)) < 0 ) {
		perror(program);
		exit(-1);
	}

	listen(sfd, 1);
	while (1) {
		int len = sizeof(client_info);
		if ( (nsfd = accept(sfd, (struct sockaddr *) &client_info, &len)) < 0 ) {
			perror(program);
			exit(-1);
		}

		if ( (pid = fork()) < 0 ) {
			perror(program);
		} else if (pid == 0) {
			close(sfd);
			serves_client(nsfd, root);
			exit(0);
		} else {
			close(nsfd);
		}
	}
}

int main(int argc, char* argv[]) {
	key_t key;
	char ip[15];
	int port;
	strcpy(ip, DEFAULT_IP);
	port = DEFAULT_PORT;
	if (argc != 2) {
		printf("usage: %s RootFolder\n", argv[0]);
		return -1;
	}
	if(access(argv[1], F_OK) == -1){
    printf("usage: root must exist \n");
    return -1;
  }
  if(access(argv[1], R_OK|W_OK) == -1){
    printf("usage: root must have rw permisions \n");
    return -1;
  }
	if ( (key = ftok("/dev/null", 65)) == (key_t) -1 ) {
		perror(argv[0]);
		return -1;
	}
	if ( (semid = semget(key, 1, 0666 | IPC_CREAT))  < 0 ) {
		perror(argv[0]);
		return -1;
	}
	semctl(semid, MUTEXWRITE, SETVAL, 1);
	server(ip, port, argv[0], argv[1]);
	return 0;
}
