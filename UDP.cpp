// udp_dt.cpp : Defines the entry point for the console application.
//
#pragma comment(lib, "wsock32.lib")
//#include "stdafx.h"
//////////////////////////////////////////////////////////////////
//UDP example program											//
//																//
//Author: Crick Lin, MOXA CS support team						//
//																//
//Date	: 09-23-2002											//
//																//
//Program descryption	:										//
//	A example program via UDP									//
//	1. hit <ESC> to stop program								//
//	2. program will send any data received from keyboard		//
//	3. program will print any data read from UDP to screen		//
//	4. Syntex: udp source_udp remote_ip remote_udp				//
//	5. Program is developed under VC++ 6.0						//
//	6. Can be used on 9X/NT/2000								//
//////////////////////////////////////////////////////////////////
#include	<winsock2.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<conio.h>
#include    <TCHAR.h>

#define IP_ERROR	0xFFFEFFFFL // Invalid ip address
#define INTERVAL	100			// 10 msec
#define RETRY		50			// connect retry count


unsigned long	    dot2ip(char* dot);
char* lib_ip2dot(unsigned long ip);


int _tmain(int argc, _TCHAR* argv[])
{
	int				port, rport, i, len;
	unsigned long	ip, rip;
	WSADATA 		wsaData;
	char			ch;
	char			buf[2048];
	char			tbuf[2048];
	char			hname[80];
	struct hostent* hent;
	SOCKET			fd, fd1;
	struct sockaddr_in	so;
	struct sockaddr_in	from, to;		/* packet received socket*/
	fd_set			readfds;
	struct timeval	authtime;


	for (i = 0; i < sizeof(tbuf); i++) {
		tbuf[i] = i % 10 + '0';
	}
	if (argc < 4) {
		printf("Syntax: %s source_udp remote_ip remoet_udp\n", argv[0]);
		return -1;
	}
	port = atoi(argv[1]);
	rip = dot2ip(argv[2]);
	rport = atoi(argv[3]);
	if (rip == IP_ERROR) {
		printf("Invalid IP address(%s)!\n", argv[2]);
		return -1;
	}
	//
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
	//
	// get host ip
	//
	gethostname(hname, sizeof(hname));
	hent = gethostbyname(hname);
	if (hent == NULL) {
		printf("can't get host!\n");
		return(-1);
	}
	ip = *(unsigned long*)hent->h_addr;
	printf("host ip address = %s,port = %d\n", (char*)lib_ip2dot(ip), port);
	//
	// create and bind udp socket
	//
	memset((char*)&so, 0, sizeof(so));
	so.sin_family = AF_INET;
	so.sin_port = htons((unsigned short)port);
	so.sin_addr.s_addr = INADDR_ANY;
	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	len = sizeof(so);
	if (bind(fd, (struct sockaddr*)&so, len) < 0) {
		printf("Can't bind to UDP port %d!\n", port);
		return(-1);
	}
	//
	// prepare remote UDP information
	//
	memset((char*)&to, 0, sizeof(to));
	to.sin_family = AF_INET;
	to.sin_port = htons((unsigned short)rport);
	to.sin_addr.s_addr = rip;

	//
	// printf received data to screen and echo back
	//
	if (fd != INVALID_SOCKET) {
		printf("This program will show all data it got from remote..\n");
		printf("hit <ESC> to stop program\n");
		for (;;) {
			if (_kbhit()) {		    // keyboard is hitted
				ch = _getch();
				if (ch == 27) {     // user hit <ESC> --> exit
					printf("\n");
					break;
				}
				i = sizeof(to);
				if ((ch == 'W') || (ch == 'w')) {
					sendto(fd, tbuf, sizeof(tbuf), 0, (sockaddr*)&to, i);
				}
				//printf("Send...\n");
				i = sizeof(to);
				sendto(fd, &ch, 1, 0, (sockaddr*)&to, i);
			}
			authtime.tv_usec = 5000L;
			authtime.tv_sec = 0;
			FD_ZERO(&readfds);
			FD_SET(fd, &readfds);
			fd1 = fd + 1;
			if (select(fd1, &readfds, NULL, NULL, &authtime) >= 0) {
				if (FD_ISSET(fd, &readfds)) {
					i = sizeof(from);
					len = recvfrom(fd, buf, sizeof(buf), 0,
						(struct sockaddr*)&from,
						&i);
					if (len > 0) {
						//						printf("read len = %d\n",len);
						//						printf("read %d bytes from %d@%s\n",len,htons(from.sin_port),(char *)lib_ip2dot(from.sin_addr.s_addr));
						//						printf("\n");
												//
												// printf in ASCII mode
												//
						for (i = 0; i < len; i++)
							printf("%c", buf[i] & 0xff);
						//					    printf("\n");
												//
												// printf in HEX mode
												//
												//for (i=0;i<len;i++)
												//	printf("%02x ,",buf[i] & 0xff);
												//printf("\n");
												//
												// echo back data
												//
						//						i = sizeof(from);
						//						sendto(fd,buf,len,0,(sockaddr *)&from,i);
					}
				}
			}
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
// convert IP to dot
//
char* lib_ip2dot(unsigned long ip)
{
	unsigned char* c;
	static char	ip_dot[20];

	c = (unsigned char*)&ip;
	sprintf_s(ip_dot, "%d.%d.%d.%d", c[0], c[1], c[2], c[3]);
	return ip_dot;
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
