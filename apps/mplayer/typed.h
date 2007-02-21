//[*]----------------------------------------------------------------------------------------------------[*]
#ifndef __TYPED_H_
#define __TYPED_H_
//[*]----------------------------------------------------------------------------------------------------[*]
// ���� Ž�� ���
typedef enum 
{
	FILE_MODE			,
	FOLDER_MODE		    ,
	FOLDER_FILE_MODE	,
	FOLDER_MP3OGG_MODE  ,
	FOLDER_AVI_MODE	    ,
	FOLDER_IMG_MODE	    ,
	FOLDER_TXT_MODE	    ,
	FOLDER_GPE_MODE	    
}SeletMode;
//[*]----------------------------------------------------------------------------------------------------[*]
// ���� ����
typedef enum 
{
	FOLDER_FORMAT	,	// ����
	MP3_FORMAT		,	// ����
	OGG_FORMAT		,
	AVI_FORMAT		,	// ������
	BMP_FORMAT		,	// �׸�
	GIF_FORMAT		,
	PNG_FORMAT		,
	JPG_FORMAT		,
	TXT_FORMAT		,	// �ؽ�Ʈ ����
	FILE_FORMAT		,	// ��Ÿ ����
	GPE_FORMAT			// GP2X ��������
}FileFormat;
//[*]----------------------------------------------------------------------------------------------------[*]
typedef enum 
{
	PLAY_STATUS,
	PAUSE_STATUS,
	STOP_STATUS,
	EXIT_STAUTS
}VideoStatus;
//[*]----------------------------------------------------------------------------------------------------[*]
typedef enum 
{
	MOVIE_VIEW,
	FILE_VIEW,
	EXIT_VIEW
}ViewMode;
//[*]----------------------------------------------------------------------------------------------------[*]
typedef struct 
{
	int 	nStartCount	;
	int 	nEndCount	;
	int 	nPosition	;
	int		nStatus		;
}SViewInfomation;
//[*]----------------------------------------------------------------------------------------------------[*]
// ���� ����Ʈ
typedef struct
{
	char 		*szName;
	FileFormat 	nAttribute;
}SFileList;
//[*]----------------------------------------------------------------------------------------------------[*]
// ���� ����
typedef struct
{
	char 		*szPath;
	SFileList 	*pList;
	int 		nCount;
}SDirInfomation;
//[*]----------------------------------------------------------------------------------------------------[*]
enum MAP_KEY
{
	VK_UP			,	// 0
	VK_UP_LEFT		,	// 1
	VK_LEFT			,	// 2
	VK_DOWN_LEFT	,	// 3
	VK_DOWN			,	// 4
	VK_DOWN_RIGHT	,	// 5
	VK_RIGHT		,	// 6
	VK_UP_RIGHT		,	// 7
	VK_START		,	// 8
	VK_SELECT		,	// 9
	VK_FL			,	// 10
	VK_FR			,	// 11	
	VK_FA			,	// 12	
	VK_FB			,	// 13	
	VK_FX			,	// 14	
	VK_FY			,	// 15	
	VK_VOL_UP		,	// 16	
	VK_VOL_DOWN		,	// 17	
	VK_TAT				// 18	
};	                	
//[*]----------------------------------------------------------------------------------------------------[*]
enum IDD_Menu
{
	PREV_FILE_BUTTON,
	NEXT_FILE_BUTTON,
	PREV_SEEK_BUTTON,
	NEXT_SEEK_BUTTON,
	PLAY_BUTTON,
	PAUSE_BUTTON,
	STOP_BUTTON,
	OPEN_BUTTON,
	EQ_BUTTON
};
//[*]----------------------------------------------------------------------------------------------------[*]
typedef struct 
{
	int x;
	int y;
}Point;
//[*]----------------------------------------------------------------------------------------------------[*]
typedef enum 
{
	NORMAL_EQ,
	CLASSIC_EQ,
	JAZZ_EQ,
	POP_EQ
}Equlize;
//[*]----------------------------------------------------------------------------------------------------[*]
typedef enum 
{
	PREV_FILE_CMD,
	NEXT_FILE_CMD,
	PREV_SEEK_CMD,
	NEXT_SEEK_CMD,
	STOP_CMD,
	OPEN_CMD,
	EXIT_CMD
}NEXT_COMMAND;
//[*]----------------------------------------------------------------------------------------------------[*]
#define true	1
#define	false	0
typedef int bool;
//[*]----------------------------------------------------------------------------------------------------[*]
#endif
//[*]----------------------------------------------------------------------------------------------------[*]
