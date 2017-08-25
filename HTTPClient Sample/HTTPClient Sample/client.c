#include <stdio.h>
#include <winsock2.h>

int main(int argc, char *argv[]){

	WSADATA wd;
	SOCKET hSocket;
	struct sockaddr_in server;
	char buf[4096];
	char *ptr;
	char *deststr;
	unsigned int **addrptr;

	if (argc != 2) {
		printf("Usage : %s dest\n", argv[0]);
		return 1;
	}
	deststr = argv[1];

	if (WSAStartup(MAKEWORD(2, 0), &wd) != 0) {
		printf("WSAStartup failed\n");
		return 1;
	}

	hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (INVALID_SOCKET == hSocket) {
		printf("socket Failed.\n");
		return 1;
	}

	server.sin_family = AF_INET;
	server.sin_port = htons(80);

	server.sin_addr.S_un.S_addr = inet_addr(deststr);
	if (server.sin_addr.S_un.S_addr == 0xffffffff) {
		struct hostent *host;

		host = gethostbyname(deststr);
		if (host == NULL) {
			if (WSAGetLastError() == WSAHOST_NOT_FOUND) {
				printf("Not Found : %s\n", deststr);
			}
			return 1;
		}

		addrptr = (unsigned int **)host->h_addr_list;

		while (*addrptr != NULL) {
			server.sin_addr.S_un.S_addr = *(*addrptr);
			if (connect(hSocket,
				(struct sockaddr *)&server,
				sizeof(server)) == 0) {
				break;
			}
			addrptr++;
		}

		if (*addrptr == NULL) {
			printf("connect : %d\n", WSAGetLastError());
			return 1;
		}
	}
	else {
		if (connect(hSocket,
			(struct sockaddr *)&server,
			sizeof(server)) != 0) {
			printf("connect : %d\n", WSAGetLastError());
			return 1;
		}
	}

	memset(buf, 0, sizeof(buf));
	_snprintf(buf, sizeof(buf), "GET / HTTP/1.0\r\n\r\n");

	int n = send(hSocket, buf, (int)strlen(buf), 0);
	if (n < 0) {
		printf("send : %d\n", WSAGetLastError());
		return 1;
	}

	memset(buf, 0, sizeof(buf));
	n = recv(hSocket, buf, sizeof(buf), 0);
	if (n < 0) {
		printf("recv : %d\n", WSAGetLastError());
		return 1;
	}
	if (strncmp("HTTP/", buf, 5) != 0) {
		printf("status line error !\n");
		fwrite(buf, n, 1, stdout);
		return 1;
	}
	if (n < 5) {
		return 1;
	}

	ptr = buf + 5;
	if (strncmp("1.0 ", ptr, 4) == 0) {
		printf("HTTP/1.0\n");
		ptr += 4;
	}
	else if (strncmp("1.1 ", ptr, 4) == 0) {
		printf("HTTP/1.1\n");
		ptr += 4;
	}
	else {
		printf("unknown HTTP version\n");
		return 1;
	}

	while (ptr < buf + n && *ptr == ' ') {
		ptr++;
	}

	if (strncmp("200 ", ptr, 4) == 0) {
		printf("code [200] OK\n");
	}
	else if (strncmp("302 ", ptr, 4) == 0) {
		printf("code [302] moved\n");
	}
	else if (strncmp("404 ", ptr, 4) == 0) {
		printf("code [404] not found\n");
	}
	else {
		printf("other status code\n");
	}

	while (ptr < buf + n && *ptr != '\n') {
		ptr++;
	}

	if (ptr < buf + n) {
		fwrite(ptr, n - (ptr - buf), 1, stdout);
	}

	while (n > 0) {
		memset(buf, 0, sizeof(buf));
		n = recv(hSocket, buf, sizeof(buf), 0);
		if (n < 0) {
			printf("recv : %d\n", WSAGetLastError());
			return 1;
		}
		fwrite(buf, n, 1, stdout);
	}

	closesocket(hSocket);
	WSACleanup();

	return 0;
}