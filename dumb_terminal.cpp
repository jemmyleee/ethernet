// dumb_t.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include "windows.h"
#include "stdio.h"
#include "conio.h"
#include "time.h"
#include "TCHAR.h"


int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE			h;
	DCB				dcb;
	char			ch;
	int				len;
	unsigned long	rlen, wlen, i;
	char			rbuf[80];
	char			tbuf[20];
	COMSTAT 		stat;
	DWORD			st;
	char            dev[80];
	int             baud = 9600;


	for (i = 0; i < sizeof(tbuf); i++)
		tbuf[i] = i % 10 + '0';
	tbuf[sizeof(tbuf) - 1] = 0x5a;

	//	tbuf[0] = 0x33;
	//	tbuf[1] = 0x00;
	//	tbuf[2] = 0x00;
	if (argc < 3) {
		printf("usage %s com baud\n", argv[0]);
		exit(0);
	}
	sprintf_s(dev, "\\\\.\\%s", argv[1]);
	baud = atoi(argv[2]);
	printf("Open %s as %d n,8,1 no flow control.\n", argv[1], baud);
	//
	// Open & initialize COM port
	//
	h = CreateFile(
		dev,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (h == (HANDLE)-1) {
		printf("Open com fail!\n");
		return 1;
	}

	dcb.DCBlength = sizeof(DCB);
	if (!GetCommState(h, &dcb)) {
		printf("GetCommState fail!\n");
		CloseHandle(h);
		return 1;
	}
	dcb.BaudRate = baud;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	//	dcb.StopBits = TWOSTOPBITS;

	dcb.fOutxCtsFlow = FALSE;
	dcb.fRtsControl = RTS_CONTROL_ENABLE;

	if (!(SetCommState(h, &dcb))) {
		printf("SetCommState fail!\n");
		CloseHandle(h);
		return 1;
	}
	PurgeComm(h, PURGE_TXCLEAR | PURGE_RXCLEAR);
	printf("Dumb terminal begin.\n");
	for (;;) {
		if (_kbhit()) {
			ch = _getch();
			if (ch == 27)
				break;

			//if ((ch == 'w') || (ch == 'W')) {
			//	printf("Send pattern begin.....\n");
			//	for (;;) {
			//		WriteFile(h,tbuf,sizeof(tbuf),&wlen,NULL);
			//		Sleep(15);
			//		if (kbhit()) {
			//			ch = getch();
			//			if (ch == 27) {
			//				printf("\nDumb terminal begin.\n");
			//				break;
			//			}
			//		}
			//	}
			//}
			PurgeComm(h, PURGE_TXCLEAR | PURGE_RXCLEAR);
			WriteFile(h, (char*)&ch, 1, &wlen, NULL);
			//			printf("\n\n");
			//			WriteFile(h,tbuf,3,&wlen,NULL);
		}
		if (ClearCommError(h, &st, &stat)) {
			len = stat.cbInQue;
			if (len >= sizeof(rbuf))
				len = sizeof(rbuf);
			if (stat.cbInQue > 0) {
				len = ReadFile(h, rbuf, len, &rlen, NULL);
				for (i = 0; i < rlen; i++) {
					printf("%c", rbuf[i]);
					//if ((rbuf[i] & 0xff) == 0x11)
					//	printf("%02x ,",rbuf[i] & 0xff);
					//else if ((rbuf[i] & 0xff) == 0x13)
					//	printf("%02x ,",rbuf[i] & 0xff);
				}
			}
		}
		Sleep(1);
	}
	//
	// process complete
	//
	printf("\nprogram exit.\n");
	CloseHandle(h);
	return 0;
	return 0;
}

