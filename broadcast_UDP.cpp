// udp_b.cpp : Defines the entry point for the console application.
//
#pragma comment(lib, "wsock32.lib")
//#include "stdafx.h"
#include	<winsock2.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<conio.h>
#include    <TCHAR.h>

int _tmain(int argc, _TCHAR* argv[])
{
	WSADATA 			wsaData;
	SOCKET				fd;
	char				hname[80];
	unsigned long		ip;
	struct	hostent* hent;
	struct sockaddr_in	so;
	int					len, i;
	struct sockaddr_in	To;
	BOOL				b = TRUE;
	char				buf[1024];

	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR) {
		printf("WSAStartup failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}
	gethostname(hname, sizeof(hname));
	hent = gethostbyname(hname);
	if (hent == NULL) {
		printf("gethostbyname fail!\n");
		WSACleanup();
		return -1;
	}
	ip = *(u_long*)hent->h_addr;

	memset((char*)&so, 0, sizeof(so));
	so.sin_family = AF_INET;
	so.sin_port = htons(4001);
	so.sin_addr.s_addr = INADDR_ANY;
	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fd == INVALID_SOCKET) {
		printf("gethostbyname fail!\n");
		WSACleanup();
		return -1;
	}
	len = sizeof(so);
	if (bind(fd, (struct sockaddr*)&so, len) < 0) {
		closesocket(fd);
		printf("bind fail!\n");
		WSACleanup();
		return -1;
	}
	if (setsockopt(fd, SOL_SOCKET,
		SO_BROADCAST, (char*)&b, sizeof(BOOL)) == SOCKET_ERROR) {
		closesocket(fd);
		printf("set broadcast fail!\n");
		WSACleanup();
		return(-1);
	}
	memset(&To, 0, sizeof(To));
	To.sin_family = AF_INET;
	To.sin_port = htons(4001);
	//	To.sin_addr.s_addr = 0xc0a8ffff;
	To.sin_addr.s_addr = 0xffffffff;
	//	To.sin_addr.s_addr = 0xffffa8c0;
	for (len = 0; len < sizeof(buf); len++)
		buf[len] = (len & 0xff);
	for (;;) {
		for (i = 0; i < 6; i++) {
			sendto(fd, buf, 60, 0, (struct sockaddr*)&To, sizeof(To));
		}
		//if (sendto(fd, buf, sizeof(buf), 0,(struct sockaddr *)&To, sizeof(To)) == SOCKET_ERROR) {
		//	printf("sendto fail!");
//			Sleep(1000);
		//}
		//Sleep(10);
	}
	closesocket(fd);
	printf("test complete.\n");
	WSACleanup();
	return 0;
}
