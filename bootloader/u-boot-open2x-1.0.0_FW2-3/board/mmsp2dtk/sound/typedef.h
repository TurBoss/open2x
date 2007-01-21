//
//	Copyright (C) 2003 MagicEyes Digital Co., All Rights Reserved
//	MagicEyes Digital Co. Proprietary & Confidential
//
//	MMSP 2.0 BASE
//	
//	- Type definition
//
//	Charlie Myung, 
//
//	history
//			2003/10/24 Jonathan typedef int S32 -> typedef long S32
//								typedef unsigned int U32 -> typedef unsigned long U32
//			2003/10/24 Jonathan MES_PUBLIC, MES_PRIVATE �߰�
//			2003/10/24 Jonathan	Windows �� ��Ÿ �÷����� �浹�� �� �ִ� type ����
//							CHAR, SHORT, INT, LONG, UCHAR, USHORT, UINT, ULONG, SINT, SSHORT, SLONG,
//							VOID
//			2003/10/22 Charlie	Sxx, Uxx�� ���� ũ��üũ �߰� (CASSERT �̿�, Nik�� ��û)
//			2003/10/21 Brian	CPU���� �ڷ� ��Ʈ���� ���̸� �غ��ϱ� ���� Sxx, Uxx type �߰�
//			2003/10/21 Charlie	LONG, ULONG �߰�, unsigned int Ÿ���� UINT�� ����, int�� INT�� ����
//			2003/10/16 Charlie	CHAR, SHORT, INT Ÿ�Կ��� signed ����
//			2003/10/06 Charlie	pDevName�� Ÿ���� char*���� const CHAR*�� ����(Nik�� ��û)
//			2003/10/.. Neo		MES_DEVINFO, MES_DEVINFO2 ����ü �߰�
//			2003/10/01 Charlie	VC���� �������ϱ� ���� ���� �߰�(Tony �ۼ�)
//			2003/09/25 Charlie
//
//

#ifndef _TYPEDEF_H
#define	_TYPEDEF_H



	typedef unsigned char   UI8;
	typedef unsigned char   byte;
	typedef signed char     SI8;
	typedef unsigned int    UI16;
	typedef unsigned int    word;
	typedef signed int      SI16;
	typedef unsigned long   UI32;



	typedef char    		S8;
	typedef short   		S16;
	typedef long   			S32;

	typedef unsigned char	U8;
	typedef unsigned short	U16;
	typedef unsigned long	U32;
	
	typedef int				CBOOL;
	
	typedef enum
	{
		CFALSE = 0,
		CTRUE  = 1
	} CBOOL_VAL;


#ifndef CNULL
#	define CNULL 0
#endif /* NULL */

#ifndef NULL
#	define NULL 0
#endif /* NULL */



#define MES_DEV_NDMA	0x0001
#define MES_DEV_FDMA	0x0002
#define MES_DEV_INT		0x0004

typedef struct _deviceinfo_ {
	const U8*	pDevName;
	U32	IOBaseAddress;
	U32	IORegionSize;
	U32	Cap;
} MES_DEVINFO;



#endif	// _TYPEDEFS_H