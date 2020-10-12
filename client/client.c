#include "header.h"

int test_case_101(int sfd) {
	int code;

	snd_msg(sfd, 101, "/imagen4_n.jpg");
	code = rcv_msg(sfd);
	printf("codigo recibido: %i\n", code);
	if (code != 201) {
		printf("TEST \"/imagen4_n.jpg\" - expected code = 201 -  received code = %i\n", code);
		return -1;
	}

	snd_msg(sfd, 101, "/noexiste.jpg");
	code = rcv_msg(sfd);
	if (code != 202) {
		printf("TEST \"/noexiste.jpg\" - expected code = 202 -  received code = %i\n", code);
		return -1;
	}

	snd_msg(sfd, 101, "imagen2_y.jpg");
	code = rcv_msg(sfd);
	if (code != 203) {
		printf("TEST \"imagen2_y.jpg\" - expected code = 203 -  received code = %i\n", code);
		return -1;
	}

	snd_msg(sfd, 101, "/dir1");
	code = rcv_msg(sfd);
	if (code != 205) {
		printf("TEST \"/dir1\" - expected code = 205 -  received code = %i\n", code);
		return -1;
	}

	snd_msg(sfd, 101, "/imagen2_y.jpg");
	code = rcv_msg(sfd);
	if (code != 301) {
		printf("TEST \"/imagen2_y.jpg\" - expected code = 301 -  received code = %i\n", code);
		return -1;
	}

	return 0;
}

int test_case_102(int sfd) {
	int code;

	snd_msg(sfd, 102, "dir1");
	code = rcv_msg(sfd);
	if (code != 203) {
		printf("TEST \"dir1\" - expected code = 203 -  received code = %i\n", code);
		return -1;
	}

	snd_msg(sfd, 102, "/noexiste");
	code = rcv_msg(sfd);
	if (code != 206) {
		printf("TEST \"/noexiste\" - expected code = 206 -  received code = %i\n", code);
		return -1;
	}

	snd_msg(sfd, 102, "/imagen2_y.jpg");
	code = rcv_msg(sfd);
	if (code != 207) {
		printf("TEST \"/imagen2_y.jpg\" - expected code = 207 -  received code = %i\n", code);
		return -1;
	}

	snd_msg(sfd, 102, "/dir1");
	code = rcv_msg(sfd);
	if (code != 302) {
		printf("TEST \"/dir1\" - expected code = 302 -  received code = %i\n", code);
		return -1;
	}

	return 0;
}


int main(int argc, char* argv[]) {
	int i, port, sfd, code;
	struct sockaddr_in server_info;

	if (argc != 3) {
	    printf("usage: %s ip port\n", argv[0]);
	    return -1;
	}

	port = atoi(argv[2]);
	if (port < 5000) {
		printf("%s: The port must be greater than 5000.\n", argv[0]);
		return -1;
	}

	if ( (sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		perror(argv[0]);
		return -1;
	}

	server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = inet_addr(argv[1]);
	server_info.sin_port = htons(port);
	if ( connect(sfd, (struct sockaddr *) &server_info, sizeof(server_info)) < 0 ) {
		perror(argv[0]);
		return -1;
	}

	code = rcv_msg(sfd);
	if (code != 1) {
		printf("TEST \"init msg\" - expected code = 1 -  received code = %i\n", code);
		close(sfd);
		exit(0);
	}

	snd_msg(sfd, 555, "NO EXISTE");
	code = rcv_msg(sfd);
	if (code != 204) {
		printf("TEST \"init msg\" - expected code = 204 -  received code = %i\n", code);
		close(sfd);
		exit(0);
	}

	if (test_case_101(sfd) != 0) {
		close(sfd);
		exit(0);
	}

	if (test_case_102(sfd) != 0) {
		close(sfd);
		exit(0);
	}

	snd_msg(sfd, 103, "Adios");

	close(sfd);
	printf("SUCCESS!!!!!!\n");
	return 0;
}
