// tcps_dt.cpp : Defines the entry point for the console application.
//

#pragma comment(lib, "wsock32.lib")
//#include "stdafx.h"
//////////////////////////////////////////////////////////
//TCP Server dumb terminal example program				//
//														//
//Author: Crick Lin, MOXA CS support team				//
//														//
//Date	: 01-23-2004									//
//														//
//Program descryption	:								//
//	A example program for tcp server					//
//	1. hit <ESC> to stop program						//
//	2. program will print any data received from remote	//
//	3. program will send any data read from keyboard	//
//	4. Syntex: tcps TCP_PORT							//
//	5. Program is developed under VC++ 6.0				//
//	6. Can be used on 9X/NT/2000						//
//////////////////////////////////////////////////////////
#include	<winsock2.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<conio.h>
#include	<io.h>
#include	<time.h>
#include	<TCHAR.h>

#define IP_ERROR	0xFFFEFFFFL // Invalid ip address
#define INTERVAL	100			// 10 msec
#define RETRY		50			// connect retry count

char* lib_ip2dot(unsigned long ip);

int _tmain(int argc, _TCHAR* argv[])
{
	int					port, i, l, len;
	unsigned long		ip;
	WSADATA 			wsaData;
	char				buf[4096], msg[512];
	char				hname[80];
	struct hostent* hent;
	SOCKET				fd, a, s;
	struct sockaddr_in	so;
	fd_set				readfds;
	struct timeval		authtime;
	int					ch;
	time_t				tt;
	struct tm			newtime;

	if (argc < 2) {
		printf("Syntax: %s TCP_Port\n", argv[0]);
		return -1;
	}
	port = atoi(argv[1]);
	// On windows we need to call WSAStartup before calling any SOCKET function
	//
	// If your project(VC++,VB,DELPHI) has include TCP/IP MODULE on it.
	//    You need not to call this function, because they do it automatically
	//    when you select TCP/IP module
	//
	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR) {
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -5;
	}

	gethostname(hname, sizeof(hname));
	hent = gethostbyname(hname);
	if (hent == NULL) {
		printf("can't get host!\n");
		return(-1);
	}
	ip = *(unsigned long*)hent->h_addr;
	printf("host ip address = %s,port = %d\n", (char*)lib_ip2dot(ip), port);
	//
	// bind source TCP port
	//
	memset((char*)&so, 0, sizeof(so));
	so.sin_family = AF_INET;
	so.sin_port = htons((unsigned short)port);
	so.sin_addr.s_addr = INADDR_ANY;
	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	len = sizeof(so);
	if (bind(fd, (struct sockaddr*)&so, len) < 0) {
		printf("Can't bind to TCP port %d!\n", port);
		return(-1);
	}
	l = -1;
	a = INVALID_SOCKET;
	l = listen(fd, 10);
	printf("listen result = %d\n", l);
	printf("accepting ...\n");
	for (;;) {
		if (_kbhit()) {
			ch = _getch();
			if (ch == 27)
				break;
			if (a != INVALID_SOCKET) {
				send(a, (char*)(&ch), 1, 0);
			}
		}
		if (a == INVALID_SOCKET) {
			a = accept(fd, 0, 0);
			if (a != INVALID_SOCKET) {
				//time(&tt);
				//newtime= localtime_s(&tt);
				//printf("accept ok (%.19s).\n",asctime(newtime));
				tt = time(NULL);
				localtime_s(&newtime, &tt);
				asctime_s(msg, sizeof(msg), &newtime);
				printf("***%s***\r\n", msg);
				len = 0;
			}
			else {
				printf("accept fail!\n");
				a = INVALID_SOCKET;
			}
			Sleep(500);
		}
		if (a != INVALID_SOCKET) {
			authtime.tv_usec = 10000L;
			authtime.tv_sec = 0;
			FD_ZERO(&readfds);
			FD_SET(a, &readfds);
			s = a + 1;
			if (select(s, &readfds, NULL, NULL, &authtime) >= 0) {
				if (FD_ISSET(a, &readfds)) {
					len = recv(a, buf, sizeof(buf), 0);
					if (len <= 0) {
						printf("Connection lost!\n");
						closesocket(a);
						a = INVALID_SOCKET;
						l = -1;
					}
					else {
						for (i = 0; i < len; i++)
							printf("%c", buf[i]);
					}
				}
			}
		}
	}
	closesocket(fd);
	//
	// On windows we need to call WSACleanup to free SOCKET resourse
	//	before program exit
	//
	WSACleanup();
	printf("hit any key to stop program...\n");
	_getch();
	printf("program exit.\n");
	return 0;
}
//
// Convert ip to dot
//
char* lib_ip2dot(unsigned long ip)
{
	unsigned char* c;
	static char	ip_dot[20];

	c = (unsigned char*)&ip;
	sprintf_s(ip_dot, "%d.%d.%d.%d", c[0], c[1], c[2], c[3]);
	return ip_dot;
}

