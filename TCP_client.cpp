// tcpc_dt.cpp : Defines the entry point for the console application.
//

//////////////////////////////////////////////////////////
//TCP client example program							//
//														//
//Author: Crick Lin, MOXA CS support team				//
//														//
//Date	: 01-23-2004									//
//														//
//Program descryption	:								//
//	A dumb terminal example program of TCP Client		//
//	1. hit <ESC> to stop program						//
//	2. program will print any data read from remote 	//
//	3. program will send any data read from keyboard    //
//  4. usage tcpc remote_ip tcp_port					//
//////////////////////////////////////////////////////////
#pragma comment(lib, "wsock32.lib")
//#include "stdafx.h"
#include	<winsock.h>
//#include	<winsock2.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<conio.h>
#include	<TCHAR.h>

#define IP_ERROR	0xFFFEFFFFL // Invalid ip address
#define INTERVAL	1			// 10  msec
#define RETRY		5000		// connect retry count

unsigned long	    dot2ip(char* dot);
SOCKET				sioopen(unsigned long ipaddr, int p);



int _tmain(int argc, _TCHAR* argv[])
{
	int				port, i;
	unsigned long	ip;
	WSADATA 		wsaData;
	char			ch, len, c;
	char			buf[80];
	SOCKET			fd, s;
	fd_set			readfds;
	struct timeval	authtime;
	unsigned long	t;

	if (argc < 3) {
		printf("Syntax: %s IP TCP_Port\n", argv[0]);
		return -1;
	}

	ip = dot2ip(argv[1]);
	if (ip == IP_ERROR) {
		printf("Invalid IP address %s!\n", argv[1]);
		return -2;
	}
	port = atoi(argv[2]);
	//
	// On windows we need to call WSAStartup before calling any SOCKET function
	//
	// If your project(VC++,VB,DELPHI) has include TCP/IP MODULE on it.
	//    You need not to call this function, because they do it automatically
	//    when you select TCP/IP module
	//
	if (WSAStartup(0x101, &wsaData) == SOCKET_ERROR) {
		fprintf(stderr, "WSAStartup failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return -5;
	}
	//
	// connect to remote
	//
	printf("connecting to --> %s@%s....", argv[2], argv[1]);
	fd = sioopen(ip, port);
	if (fd != INVALID_SOCKET) {
		printf("ok\n");
		printf("<ESC> = stop program.\n");
		t = 0;
		c = '0';
		for (;;) {
			if (_kbhit()) {		    // keyboard is hitted
				ch = _getch();
				if (ch == 27) {	    // user hit <ESC> --> exit
					printf("\n");
					break;
				}
				send(fd, &ch, 1, 0);
			}
			//if ((GetCurrentTime() - t) >= 2000) { // send 1 bytes per 2 seconds
			//	send(fd,&c,1,0);
			//	c++;
			//	if (c > '9')
			//		c = '0';
			//	t = GetCurrentTime();
			//}
			authtime.tv_usec = 10000L;
			authtime.tv_sec = 0;
			FD_ZERO(&readfds);
			FD_SET(fd, &readfds);
			s = fd + 1;
			if (select(s, &readfds, NULL, NULL, &authtime) >= 0) {
				if (!(FD_ISSET(fd, &readfds))) {
					continue;
				}
			}
			else {
				continue;
			}

			len = recv(fd, buf, sizeof(buf), 0);

			if (len <= 0) {	// No data read
				printf("connection lost !\n");
				break;
			}
			else {
				for (i = 0; i < len; i++)
					printf("%c", buf[i] & 0xff);
			}
			Sleep(300);
		}
		closesocket(fd);	// Close TCP connection
	}
	else {
		printf("fail!\n");
	}
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
//Convert dot notation to IP address
// ie : Form "192.168.2.1" to 0x0102A8C0
//
unsigned long dot2ip(char* dot)
{
	unsigned long	ip;
	unsigned char* c;
	int		i, d;

	c = (unsigned char*)&ip;
	for (i = 4; i-- > 0; ) {
		d = *dot++ - '0';
		if (d < 0 || d > 9)
			return IP_ERROR;
		while (*dot >= '0' && *dot <= '9') {
			d = d * 10 + *dot++ - '0';
			if (d > 255)
				return IP_ERROR;
		}
		*c++ = d;
		if (*dot++ != '.')
			break;
	}
	if (*--dot || i)
		return IP_ERROR;
	return ip;
}
//
//Connect to remote tcp port
//
SOCKET sioopen(unsigned long ipaddr, int port)
{
	struct sockaddr_in	des;
	int					i, j, len;
	SOCKET				fd;
	BOOL				b = TRUE;
	ULONG				mode = 1;   /* set to non_delay mode */
	unsigned short		p;
	DWORD				t, t1;
	unsigned char		ch = 0xc0;

	p = htons((unsigned short)port);
	//
	// open socket
	//
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == INVALID_SOCKET) {
		return(fd);
	}
	//	if (setsockopt(fd,IPPROTO_IP,IP_TOS,(char *)&ch,1)) {
	//		printf("set tos fail!\n");
	//	}
	//
	// Set SOCKET to No Delay mode
	//
	//	if (ioctlsocket(fd,FIONBIO,&mode)) {
	//	    closesocket(fd);
	//	    return(INVALID_SOCKET);
	//	}
	//
	// Set remote IP address and port no
	//
	des.sin_family = AF_INET;
	des.sin_addr.s_addr = ipaddr;
	des.sin_port = p;
	len = sizeof(struct sockaddr_in);
	//
	// connect to remote
	//
	i = 0;
	t = GetCurrentTime();
	for (;;) {
		j = connect(fd, (struct sockaddr*)&des, len);
		if (j == 0)				// connected
			break;
		if (WSAGetLastError() == WSAEISCONN) {	// already connected
			j = 0;
			break;
		}
		if (i++ >= RETRY)   // Connect fail too many times --> give up
			break;

		Sleep(INTERVAL);// Sleep for a while before trying it again.
		// Prevent from wasting too much of CPU time.
	}
	t1 = GetCurrentTime();
	printf("Connect time = %d msec.\n", t1 - t);
	if (j != 0) {		// Can't connect to remote
		closesocket(fd);
		return(INVALID_SOCKET);
	}
	//	if (setsockopt(fd,IPPROTO_IP,IP_TOS,(char *)&ch,1)) {
	//		printf("set tos fail(%d)!\n",WSAGetLastError());
	//	}
	return(fd);
}


