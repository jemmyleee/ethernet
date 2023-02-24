// Multicast_udp.cpp : 此檔案包含 'main' 函式。程式會於該處開始執行及結束執行。
//

#include <iostream>
// multicast_t.cpp : Defines the entry point for the console application.
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
//#include	<Ws2tcpip.h>

#define IP_ERROR	0xFFFEFFFFL // Invalid ip address
#define INTERVAL	100			// 10 msec
#define RETRY		50			// connect retry count

unsigned long	    dot2ip(char* dot);
char* lib_ip2dot(unsigned long ip);


//#define	IP_MULTICAST_TTL    10  /* set/get IP multicast timetolive */
//#define IP_MULTICAST_LOOP   11  /* set/get IP multicast loopback */
//#define	IP_ADD_MEMBERSHIP   12  /* add  (set) IP group membership */
#define IP_TOS			3   /* old (winsock 1.1) value 8 */
#define IP_TTL			4   /* old value 7 */
#define IP_MULTICAST_IF		9   /* old value 2 */
#define IP_MULTICAST_TTL	10  /* old value 3 */
#define IP_MULTICAST_LOOP	11  /* old value 4 */
#define IP_ADD_MEMBERSHIP	12  /* old value 5 */
#define IP_DROP_MEMBERSHIP	13  /* old value 6 */
#define IP_DONTFRAGMENT		14  /* old value 9 */
#define IP_ADD_SOURCE_MEMBERSHIP	15
#define IP_DROP_SOURCE_MEMBERSHIP	16
#define IP_BLOCK_SOURCE			17
#define IP_UNBLOCK_SOURCE		18
#define IP_PKTINFO			19



typedef struct ip_mreq {
	struct in_addr imr_multiaddr;
	struct in_addr imr_interface;
} IP_MREQ, * PIP_MREQ;


int	SendStream = 0;
void Phelp(void)
{
	printf("------------------------------------\r\n");
	printf("Multicase stream sending = %d\r\n", SendStream);
	printf(" 1 : send join \r\n");
	printf(" 2 : send leave \r\n");
	printf(" 3 : Start sending multicast stream.\r\n");
	printf(" 4 : Stop sending multicast stream.\r\n");
	printf("-----------------------------------\r\n");

}


int _tmain(int argc, _TCHAR* argv[])
{
	int				rport, i, ret;
	unsigned long	ip, rip;
	WSADATA 		wsaData;
	char			ch, len;
	char			buf[2048];
	char			hname[80];
	struct hostent* hent;
	SOCKET			fd, fd1;
	struct sockaddr_in	so;
	struct sockaddr_in	from, to;		/* packet received socket*/
	fd_set			readfds;
	struct timeval	authtime;
	IP_MREQ			req;
	char			flag = 0;
	char			ttl = 5;
	unsigned long	t;

	if (argc < 3) {
		printf("Syntax: %s multicast_ip port\n", argv[0]);
		return -1;
	}
	rip = dot2ip(argv[1]);
	rport = atoi(argv[2]);
	if (rip == IP_ERROR) {
		printf("Invalid IP address(%s)!\n", argv[2]);
		return -1;
	}
	if (((rip & 0xff) < 224) || ((rip & 0xff) > 239)) {
		printf("Multicast IP address must be between 224.x.x.x and 239.x.x.x !(%x)\n", rip & 0xff);
		return -2;
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
	printf("Multicast ip address = %s,port = %d\n", (char*)lib_ip2dot(rip), rport);
	//
	// create and bind udp socket
	//
	memset((char*)&so, 0, sizeof(so));
	so.sin_family = AF_INET;
	so.sin_port = htons((unsigned short)rport);
	so.sin_addr.s_addr = INADDR_ANY;
	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	len = sizeof(so);
	if (bind(fd, (struct sockaddr*)&so, len) < 0) {
		printf("Can't bind to UDP port %d!\n", rport);
		return(-1);
	}
	req.imr_interface.s_addr = NULL;
	req.imr_multiaddr.s_addr = rip;
	//	ret = setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char *)&req,sizeof(req));
	//	if (ret == SOCKET_ERROR) {
	//		printf("IP_ADD_MEMBER fail!\n");
	//	}
	//	ret = setsockopt(fd,IPPROTO_IP,IP_ADD_SOURCE_MEMBERSHIP,(char *)&req,sizeof(req));
	//	if (ret == SOCKET_ERROR) {
	//		printf("IP_ADD_SOURCE_MEMBERSHIP fail!\n");
	//	}

	ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof(flag));
	if (ret == SOCKET_ERROR)
		printf("IP_MULTICAST_LOOP fail!\n");

	ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
	if (ret == SOCKET_ERROR)
		printf("IP_MULTICAST_TTL fail!\n");


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
		t = 0;
		Phelp();
		for (;;) {
			if (_kbhit()) {		    // keyboard is hitted
				ch = _getch();
				if (ch == 27) {     // user hit <ESC> --> exit
					printf("\n");
					break;
				}
				//printf("Send...\n");
								//i = sizeof(to);
								//sendto(fd,&ch,1,0,(sockaddr *)&to,i);
				switch (ch) {
				case '1':
					ret = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&req, sizeof(req));
					if (ret == SOCKET_ERROR) {
						printf("IP_ADD_MEMBER fail!\r\n");
					}
					else {
						printf("IP_ADD_MEMBER ok.\r\n");
					}
					break;
				case '2':
					ret = setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&req, sizeof(req));
					if (ret == SOCKET_ERROR) {
						printf("IP_DROP_MEMBERSHIP fail!\r\n");
					}
					else {
						printf("IP_DROP_MEMBERSHIP ok.\r\n");
					}
					break;
				case '3':
					SendStream = 1;
					break;
				case '4':
					SendStream = 0;
					break;
				case '?':
				default:
					Phelp();
					break;
				}
			}
			if ((GetCurrentTime() - t) >= 1000) {
				if (SendStream) {
					i = sizeof(to);
					sendto(fd, "1234567890\r\n", 12, 0, (sockaddr*)&to, i);
				}
				t = GetCurrentTime();
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
		ret = setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&req, sizeof(req));
		if (ret == SOCKET_ERROR) {
			printf("IP_DROP_MEMBERSHIP fail!\n");
		}
		//		ret = setsockopt(fd,IPPROTO_IP,IP_DROP_SOURCE_MEMBERSHIP,(char *)&req,sizeof(req));
		//		if (ret == SOCKET_ERROR) {
		//			printf("IP_DROP_SOURCE_MEMBERSHIP fail!\n");
		//		}
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
