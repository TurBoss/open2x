/*              
 *  - godori <ghcstop>, www.aesop-embedded.org
 *             
 *    => Created. October, 2004
 *
 *    => DIGNSYS Inc. < www.dignsys.com > developing from April 2005
 *
 */

#include <SDL.h>
#include <SDL_thread.h>
#include <sys/stat.h>

#include "gvlib_export.h"
#include "fbdisp/gfxdev.h"
#include "fbdisp/fbs.h"
#include "libao2/audio_out.h"		

#include "cx25874.h"		

#include "typed.h"
#include "filelistview.h"
#include "SDL_inifile.h"

#define IMG_RESUME	"./skin/resume.png"

char charset[256];
int leftgap, rightgap, topgap, bottomgap, minbottom;
int font_red, font_green, font_blue;
int outline_red, outline_green, outline_blue;
extern int resize_mode;

SDL_Surface 	*g_pScreen 			= NULL;		
SDL_Joystick 	*m_pJoy				= NULL;		
SDL_Surface 	*g_pImg_Body		= NULL;		
int 			done				= 0;		

Uint8			g_Joystate			= 0;			
Uint8			g_LastStick			= 0;			
Uint32 			g_DownTime			= 0;			

int			NowTick				= 0;
int 		TotalPlayTime		= 0;
float 		CurrentPlayTime		= 0.0f;
float 		g_fResumePlayTime	= 0.0f;
bool 		g_bResumePlay		= false;

int 		g_Kbps			= 0;
int 		g_Khz			= 0;
char 		*g_pTagTitle 	= NULL;
bool 		g_stero			= false;
bool 		g_repeate		= false;
bool 		g_shuf			= false;
Equlize 	g_eq			= NORMAL_EQ;

extern ao_functions_t    	*audio_out;
extern int 					pause_flag;

int 			quit_event 			= 0;	
int 			break_signal_sent 	= 0;	

unsigned int    disp_mode 			= 0; 	


int 			load_count 			= 0;	
VideoFile 		vf;
char    		*ifile 				= NULL;			
bool			bFileSelect			= false;		
NEXT_COMMAND	g_Command 			= NEXT_FILE_CMD;			


int 			g_FBStatus			= 0x01;			
int				posCursor			= 0;			
ViewMode		viewmode			= FILE_VIEW;	
ViewMode		oldviewmode			= FILE_VIEW;
bool			bMenuStatus			= false;
bool 			g_NotSupported		= false;

bool			bSeekPrevButton		= false;
bool			bSeekNextButton		= false;
int				g_ProgressValue		= 0;

bool			bSeekPrevStick		= false;
bool			bSeekNextStick		= false;
int				g_SeekTime			= 0;

bool			g_bHoldLCDoff		= false;
int				g_bTVMode			= 0;

static int 		sub_mode 			= 0; 
VideoStatus 	vstate 					= STOP_STATUS;

	//senquack - no longer used
int				cpuclock			= 0;

extern int force_backward_seek;
extern int leftVol;

extern int 				set_of_sub_size; 
extern SViewInfomation 	infoView;					
extern SDirInfomation	infoDir;					

void init_SDL_engine(void);				
void InitApp();
void ExitApp();
void event_loop(void);					
int SetMode(int argc, char *argv[]);
void OnExitClick();

extern void volume_change(bool up_flag);

void InitApp()
{
   	
	infoView.nStartCount	= 0;
   	infoView.nEndCount		= 3;
   	infoView.nPosition		= 0;
   	infoView.nStatus		= 0;
   	infoDir.nCount			= 3;

	g_Joystate	= 0;
	g_LastStick	= 0;
	g_DownTime	= 0;
}

void ExitApp()
{
    FileView_Exitialize();		
    if(ifile != NULL)	free(ifile);	
    DeleteDirInfo(&infoDir);		

	if(g_FBStatus == 0x02)	
		switch_fb1_to_fb0();
	
	
	if(SDL_JoystickOpened(0))	
		SDL_JoystickClose(m_pJoy);
	
	SDL_Quit();

	//senquack - we'll handle this in our own script
//	chdir("/usr/gp2x/");		// 파일이 있는 디렉토리로 이동
//	execlp("./gp2xmenu", "./gp2xmenu", NULL, 0);		// 파일 실행
}

int main(int argc, char *argv[])
{
		//senquack - GPH is so amateur!  This doesn't work after the
		//executable is already loaded, it is pointless.
//	{
//		char libpath[1024];
//		struct stat statbuf;
//
//		getcwd(libpath, 1000);
//		strcat(libpath, "/libiconv_plug.so");
//		if(!lstat(libpath, &statbuf) && S_ISREG(statbuf.st_mode))
//			setenv("LD_PRELOAD", libpath, 0);
//
    init_SDL_engine();
	if(SetMode(argc, argv) != 0)
	{
		printf("Display Mode Set failed!!\n");
		ExitApp();
		return -1;
	}

    InitApp();					
    init_etc_setting();			
    osd_plane_open();			
	
	viewmode = FILE_VIEW;
    FileView_Initialize();			
	FileView_OnDraw(g_pScreen);		

    event_loop();               
    
	 //	senquack - this is now in the same folder as the binary for open2x:
//	INI_Open("/usr/gp2x/common.ini");
	INI_Open("common.ini");
	INI_WriteInt("sound", "volume", leftVol);
	INI_WriteInt("video", "speed", cpuclock);
	INI_Close();

	osd_plane_close();			
    close_etc_setting();		
    
    ExitApp();					

    return 1;
}


void FileEvnetLoop(SDL_Event *event, SDL_Event *open_event)
{
	switch(event->type)
	{
		case SDL_KEYDOWN:				
			if(event->key.keysym.sym == SDLK_b)
			{
				OnExitClick();
				return;
			}
			break;
		case SDL_JOYBUTTONDOWN :		
			if(event->jbutton.button == VK_START)
			{
				OnExitClick();
				return;
			}
			break;
	}
	FileView_OnProc(g_pScreen, event, open_event);

	
	if(event->type == SDL_JOYBUTTONDOWN) 
	{
		switch(event->jbutton.button)
		{
			case VK_UP 	  :
			case VK_DOWN  :
				g_Joystate	= SDL_JOYBUTTONDOWN;
				g_LastStick	= event->jbutton.button;
				g_DownTime	= SDL_GetTicks();
				break;
		}
	}
	else if(event->type == SDL_JOYBUTTONUP) 
	{
		g_Joystate = SDL_JOYBUTTONUP;
	}
}


void OnRelativeSeek(double sec)
{
	movie_seek(sec, RELATIVE_SEEK_BY_SECOND);
}


void OnAbsoluteSeek(double sec)
{
	movie_seek(sec, ABSOLUTE_SEEK_BY_POSITION);
}

int GetPause()
{
	return pause_flag;
}

void SetPause(int status)
{
	PauseTimer(status);
	if(!status) {
		force_backward_seek = 1;
		OnRelativeSeek(-1);
	}
	pause_flag = status;
}


void OnPrevSong()
{
	if(pause_flag) SetPause(false);
	if(infoView.nPosition-1 >= 0)	
	{
		
		if(infoDir.pList[infoView.nPosition-1].nAttribute == AVI_FORMAT)
		{
			pause_flag 	= 0;			
			quit_event 	= 1;			
			g_Command = PREV_FILE_CMD;	
		}
	}
}


void OnNextSong()
{
	if(pause_flag) SetPause(false);
	if(infoView.nPosition+1<infoDir.nCount)	
	{
		
		if(infoDir.pList[infoView.nPosition+1].nAttribute == AVI_FORMAT)
		{
			pause_flag 	= 0;			
			quit_event 	= 1;			
			g_Command = NEXT_FILE_CMD;	
		}
	}
}


void OnPrevSeek()
{
	OnRelativeSeek(-10.0);	
}


void OnNextSeek()
{
	OnRelativeSeek(10.0);	
}

void SaveResumeInfo()
{
	//senquack - now in same path as binary for open2x
//	INI_Open("/usr/gp2x/movie.ini");
	INI_Open("movie.ini");
	INI_WriteFloat("resume", "time", CurrentPlayTime);
	INI_WriteText("resume", "path", infoDir.szPath);
	INI_WriteText("resume", "file", infoDir.pList[infoView.nPosition].szName);
	INI_Close();

	sync();
}

void OnToolBar_Play(SDL_Event *open_event)
{
	
	if(GetPause())		
	{
		SetPause(false);	
		OnAbsoluteSeek((double)CurrentPlayTime/(double)TotalPlayTime*100.0);	
	}
	else if(vstate == STOP_STATUS)			
	{
		if(bFileSelect == true)	
		{
			viewmode 	= MOVIE_VIEW;	
			bFileSelect = true;		
	
		    if(ifile != NULL)	free(ifile);	
		    ifile = (char*)malloc(strlen(infoDir.szPath) + strlen(infoDir.pList[infoView.nPosition].szName) + 2);			
			sprintf(ifile, "%s/%s", infoDir.szPath, infoDir.pList[infoView.nPosition].szName);		
		    open_event->type = EVENT_MOVIE_PLAY;		
		    open_event->user.data1 = (void*)ifile;		
		    SDL_PushEvent(open_event);					
		}
	}
}


void OnToolBar_Stop()
{
	pause_flag 	= 0;		
	quit_event 	= 1;		
	g_Command = STOP_CMD;	
}


void OnToolBar_Pause()
{
	if(vstate != PLAY_STATUS)	
	{
		printf("vstate != PLAY_STATUS OK \n");
		return;
	}
	else
	{
		printf("vstate != PLAY_STATUS NOT OK \n");
	}
	
	if(GetPause() == true)	
	{
		SetPause(false);	
		OnAbsoluteSeek((double)CurrentPlayTime/(double)TotalPlayTime*100.0);	
	}
	else
	{
        SetPause(true);		
        audio_out->reset(); 
	}
}

void OnToolBar_Open()
{
	if(vstate == STOP_STATUS)			
	{
		switch_fb1_to_fb0();
		viewmode = FILE_VIEW;
	    FileView_Initialize();			
		FileView_OnDraw(g_pScreen);		
	}
	else
	{
		pause_flag 	= 0;		
		quit_event 	= 1;		
		g_Command = OPEN_CMD;	
	}
}

void OnToolBarEvent(SDL_Event *open_event)
{
	if(!bMenuStatus)	return;	
	
	switch(posCursor)	
	{
		case PREV_FILE_BUTTON :	
			OnPrevSong();
			break;
		case NEXT_FILE_BUTTON :	
			OnNextSong();
			break;
		case PREV_SEEK_BUTTON :	
			if(vstate == PLAY_STATUS)
			{
				bSeekPrevButton = true;
				g_ProgressValue = (double)CurrentPlayTime/(double)TotalPlayTime*100.0;
				if(bSeekPrevStick == true || bSeekNextStick == true)
					bSeekPrevStick = bSeekNextStick = false;
				else
				NowTick = g_SeekTime = SDL_GetTicks();		
			}
            
			break;
		case NEXT_SEEK_BUTTON :	
			if(vstate == PLAY_STATUS)
			{
	 			bSeekNextButton = true;
				g_ProgressValue = (double)CurrentPlayTime/(double)TotalPlayTime*100.0;
				if(bSeekPrevStick == true || bSeekNextStick == true)
					bSeekPrevStick = bSeekNextStick = false;
				else
				NowTick = g_SeekTime = SDL_GetTicks();		
	 		}
            
			break;
		case PLAY_BUTTON :		
			OnToolBar_Play(open_event);
			break;
		case PAUSE_BUTTON :		
			OnToolBar_Pause();
			break;
		case STOP_BUTTON :		
			OnToolBar_Stop();
			break;
		case OPEN_BUTTON :		
			OnToolBar_Open();
			break;
	}
}

/*
void OnToolBar()
{
	if(bMenuStatus == true)	HideToolBar();
	else					ShowToolBar();	
	
}
*/

void OnMoveToolBarIcon(int arrow)
{
	int oldpos = posCursor;		
	if(arrow == -1)
	{
		posCursor = (posCursor-1+8)%8;
	}
	else if(arrow == 1)
	{	
		
		posCursor = (posCursor+1+8)%8;
	}
	
	if(oldpos != posCursor)	OnDraw_MoveMenu(oldpos, posCursor); 
		
}


void OnExitClick()
{
	g_Command = EXIT_CMD;
	pause_flag 	= 0;		
	quit_event 	= 1;			
	
	if(vstate == STOP_STATUS)
	{
		if(g_FBStatus == 0x02)	
			switch_fb1_to_fb0();
		done = 1;
	}

	
}


void OnJoystickDown(Uint8 button, SDL_Event *open_event)
{
	if(!bMenuStatus)		
	{
		switch(button)
		{
			case VK_LEFT 	:		
				if(vstate == PLAY_STATUS)
				{
					NowTick = g_SeekTime = SDL_GetTicks();		
					g_ProgressValue = (double)CurrentPlayTime/(double)TotalPlayTime*100.0;
					bSeekPrevStick = true;
				}
				
				break;		
			case VK_RIGHT	:		
				if(vstate == PLAY_STATUS)
				{
					NowTick = g_SeekTime = SDL_GetTicks();		
					g_ProgressValue = (double)CurrentPlayTime/(double)TotalPlayTime*100.0;
					bSeekNextStick = true;
				}
				
				break;		
			case VK_FL 	 	:		
				OnPrevSong();				
				break;		
			case VK_FR 	 	:		
				OnNextSong();				
				break;		
			case VK_FA   	:								
				break;
			case VK_TAT   	:	
				OnToolBar_Pause();
				break;
			case VK_FB   	:	

				break;
			case VK_FY 	 	:		
				ShowMenu();					
				break;		
			case VK_FX 	 	:		
				if(vstate == PLAY_STATUS)	
				{
					OnToolBar_Stop();	
				}
				else					
				{
					switch_fb1_to_fb0();
					viewmode = FILE_VIEW;
				    FileView_Initialize();			
					FileView_OnDraw(g_pScreen);		
					
				}
				break;		
		}
	}
	else					
	{
		switch(button)
		{
			case VK_LEFT 	:		
				OnMoveToolBarIcon(-1);		
				break;		
			case VK_RIGHT	:		
				OnMoveToolBarIcon(1);		
				break;		
	                                                        
			case VK_FL 	 	:		
				OnPrevSong();				
				break;		
			case VK_FR 	 	:		
				OnNextSong();				
				break;      

			case VK_FA   	:								
				break;

			case VK_TAT   	:	
				
				
			case VK_FB   	:	
				OnToolBarEvent(open_event);	
				break;
			case VK_FY 	 	:		
				HideMenu();					
				break;		
			case VK_FX 	 	:								
				if(vstate == PLAY_STATUS)	OnToolBar_Stop();	
				else					OnToolBar_Open();	
				break;		
		}
	}
}

void OnJoystickUp(Uint8 button, SDL_Event *open_event)
{
	if(bMenuStatus)		
	{
		switch(button)
		{
			case VK_LEFT :
				if(bSeekPrevStick == true || bSeekNextStick == true)
				{
					bSeekPrevStick = false;
					bSeekNextStick = false;
					if(SDL_GetTicks()-g_SeekTime < 500)	OnPrevSeek();
					else								OnAbsoluteSeek((double)g_ProgressValue);
				}
				break;		
			case VK_RIGHT :
				if(bSeekPrevStick == true || bSeekNextStick == true)
				{
					bSeekPrevStick = false;
					bSeekNextStick = false;
					if(SDL_GetTicks()-g_SeekTime < 500)	OnNextSeek();
					else								OnAbsoluteSeek((double)g_ProgressValue);
				}
				break;
			case VK_TAT :	
			case VK_FB :	
				printf("OnJoystickUp\n");
				if(bSeekPrevButton == true)			
				{
					printf("OnJoystickUp bSeekPrevButton \n");

					bSeekPrevButton		= false;
					bSeekNextButton		= false;
					if(SDL_GetTicks()-g_SeekTime < 500)	OnPrevSeek();
					else								OnAbsoluteSeek((double)g_ProgressValue);
				}
				else if(bSeekNextButton == true)		
				{
					printf("OnJoystickUp bSeekPrevButton \n");

					bSeekPrevButton		= false;
					bSeekNextButton		= false;
					if(SDL_GetTicks()-g_SeekTime < 500)	OnNextSeek();
					else								OnAbsoluteSeek((double)g_ProgressValue);
				}
		}
	}
	else		
	{
		switch(button)
		{
			
			
			
			
			case VK_LEFT :
				if(bSeekPrevStick == true || bSeekNextStick == true)
				{
					bSeekPrevStick = false;
					bSeekNextStick = false;
					if(SDL_GetTicks()-g_SeekTime < 500)	OnPrevSeek();
					else								OnAbsoluteSeek((double)g_ProgressValue);
					HideMenu();
				}
				break;		
			case VK_RIGHT :	
				if(bSeekPrevStick == true || bSeekNextStick == true)
				{
					bSeekPrevStick = false;
					bSeekNextStick = false;
					if(SDL_GetTicks()-g_SeekTime < 500)	OnNextSeek();
					else								OnAbsoluteSeek((double)g_ProgressValue);
					HideMenu();
				}
				break;
			case VK_TAT :
			case VK_FB :
				if(bSeekPrevButton == true)			
				{
					bSeekPrevButton		= false;
					bSeekNextButton		= false;
					if(SDL_GetTicks()-g_SeekTime < 500)	OnPrevSeek();
					else								OnAbsoluteSeek((double)g_ProgressValue);
				}
				else if(bSeekNextButton == true)		
				{
					bSeekPrevButton		= false;
					bSeekNextButton		= false;
					if(SDL_GetTicks()-g_SeekTime < 500)	OnNextSeek();
					else								OnAbsoluteSeek((double)g_ProgressValue);
				}
		}
	}
}

void OnKeyDown(SDLKey key, SDL_Event *open_event)
{
	printf("KEY DOWN\n");
	if(!bMenuStatus)		
	{
		switch(key)
		{
			case SDLK_j 	:	OnPrevSeek();				break;		
			case SDLK_l		:	OnNextSeek();				break;		
	                         
			case SDLK_9	 	:	OnPrevSong();				break;		
			case SDLK_0	 	:	OnNextSong();				break;		
	                         
			case SDLK_n   	:								break;
			case SDLK_u   	:	OnToolBarEvent(open_event);	break;
			case SDLK_m	 	:	ShowMenu();					break;		
			case SDLK_y	 	:								break;		
	                         
			case SDLK_b 	:	OnExitClick();				break;
			case SDLK_z		: 	volume_change(false);		break;
			case SDLK_x		: 	volume_change(true);		break;
		}
	}
	else					
	{
		switch(key)
		{
			case SDLK_j 	:	OnMoveToolBarIcon(-1);		break;		
			case SDLK_l		:	OnMoveToolBarIcon(1);		break;		
	                         
			case SDLK_9	 	:	OnPrevSong();				break;		
			case SDLK_0	 	:	OnNextSong();				break;      
	                         
			case SDLK_n   	:								break;
			case SDLK_u   	:	OnToolBarEvent(open_event);	break;
			case SDLK_m	 	:	HideMenu();					break;		
			case SDLK_y	 	:								break;		
	                         
			case SDLK_b     :	OnExitClick();				break;
			case SDLK_z		: 	volume_change(false);		break;
			case SDLK_x		: 	volume_change(true);		break;
		}
	}
}

void OnKeyUp(SDLKey key, SDL_Event *open_event)
{
}

void ShowNotSupported()
{
	SDL_Surface *imgLoading = NULL;
	SDL_Rect imgrect;

	
	imgLoading = IMG_Load("./skin/error.png");
	
	if(imgLoading != NULL)
	{
		imgrect.x = (320-imgLoading->w)/2;
		imgrect.y = (240-imgLoading->h)/2;
		imgrect.w = imgLoading->w;
		imgrect.h = imgLoading->h;
		
		SDL_BlitSurface(imgLoading, NULL, g_pScreen, &imgrect);
		if(imgLoading)	SDL_FreeSurface(imgLoading);

		//senquack
		SDL_UpdateRect(g_pScreen, imgrect.x, imgrect.y, imgrect.w, imgrect.h);
//		SDL_Flip(g_pScreen);

	}
}
void ShowLoading()
{
	SDL_Surface *imgLoading = NULL;
	SDL_Rect imgrect;

	
	imgLoading = IMG_Load("./skin/loading.png");
	
	if(imgLoading != NULL)
	{
		imgrect.x = (320-imgLoading->w)/2;
		imgrect.y = (240-imgLoading->h)/2;
		imgrect.w = imgLoading->w;
		imgrect.h = imgLoading->h;
		
		SDL_BlitSurface(imgLoading, NULL, g_pScreen, &imgrect);
		if(imgLoading)	SDL_FreeSurface(imgLoading);

		//senquack
		SDL_UpdateRect(g_pScreen, imgrect.x, imgrect.y, imgrect.w, imgrect.h);
//		SDL_Flip(g_pScreen);
	}
}

void OnPlay()
{
	SDL_Rect dstrect = { 0, 0, 320, 240 };
	
	
	pause_flag 			= 0;	
    break_signal_sent 	= 0;	
	quit_event 			= 0;	

	bSeekPrevButton		= false;
	bSeekNextButton		= false;
	bSeekPrevButton		= false;
	bSeekNextButton		= false;
	g_ProgressValue		= 0;

	
	if(g_FBStatus == 0x02)	switch_fb1_to_fb0();
	
	
	SDL_FillRect(g_pScreen, &dstrect, SDL_MapRGB(g_pScreen->format, 0x00, 0x00, 0x00));

	//senquack
	SDL_UpdateRect(g_pScreen, dstrect.x, dstrect.y, dstrect.w, dstrect.h);
//		SDL_Flip(g_pScreen);

	ShowLoading();
	
	vstate = PLAY_STATUS;		
	viewmode = MOVIE_VIEW;	
	printf("play file name : %s\n", ifile);
	
	CurrentPlayTime = 0;			
	NowTick = SDL_GetTicks();		
	
	if(init_media_play(ifile) < 0)    
	{
	    printf("media play initialize error\n");
	    quit_event = 0;
	    break_signal_sent = 0;
		vstate = STOP_STATUS;
				
		ShowNotSupported();
		return;
	}

	
	SDL_FillRect(g_pScreen, &dstrect, SDL_MapRGB(g_pScreen->format, 0x00, 0x00, 0x00));

	//senquack
	 SDL_UpdateRect(g_pScreen, dstrect.x, dstrect.y, dstrect.w, dstrect.h);
//		SDL_Flip(g_pScreen);

	
	switch_fb0_to_fb1();

	
	if(!set_of_sub_size)	sub_mode = 0;
	else					sub_mode = 1;
	
	HideMenu();
}

void OnFileOpen()
{
	printf("EVENT_FILE_OPEN\n");
}


void OnQuitPlay(SDL_Event *open_event)
{
    quit_event = 1;
    printf("EVENT_QUIT_PLAY occur\n");
    exit_media_play();

	vstate = STOP_STATUS;			
	
    quit_event = 0;
    break_signal_sent = 0;
    
    if(g_Command == PREV_FILE_CMD)	
    {
		
		if(infoView.nPosition-1 >= 0)	
		{
			
			if(infoDir.pList[infoView.nPosition-1].nAttribute == AVI_FORMAT)
			{
				infoView.nPosition--; 		
				viewmode = MOVIE_VIEW;		
				bFileSelect = true;			
				g_Command = NEXT_FILE_CMD;	
		
				
			    if(ifile != NULL)	free(ifile);	
			    ifile = (char*)malloc(strlen(infoDir.szPath) + strlen(infoDir.pList[infoView.nPosition].szName) + 2);
				sprintf(ifile, "%s/%s", infoDir.szPath, infoDir.pList[infoView.nPosition].szName);	
			    open_event->type = EVENT_MOVIE_PLAY;
			    open_event->user.data1 = (void*)ifile;
			    SDL_PushEvent(open_event);
			    printf("PRev File Play\n");
			}
			else    
			{
				g_Command = STOP_CMD;	
			}
		}
		else
		{
			g_Command = STOP_CMD;	
		}
    }
    else if(g_Command == NEXT_FILE_CMD)	
    {
		
		if(infoView.nPosition+1<infoDir.nCount)	
		{
			
			if(infoDir.pList[infoView.nPosition+1].nAttribute == AVI_FORMAT)
			{
				infoView.nPosition++; 	
				viewmode = MOVIE_VIEW;	
				bFileSelect = true;		
				g_Command = NEXT_FILE_CMD;	
		
				
			    if(ifile != NULL)	free(ifile);	
			    ifile = (char*)malloc(strlen(infoDir.szPath) + strlen(infoDir.pList[infoView.nPosition].szName) + 2);
				sprintf(ifile, "%s/%s", infoDir.szPath, infoDir.pList[infoView.nPosition].szName);	
			    open_event->type = EVENT_MOVIE_PLAY;
			    open_event->user.data1 = (void*)ifile;
			    SDL_PushEvent(open_event);
			    printf("PRev File Play\n");
			}
			else    
			{
				g_Command = STOP_CMD;	
			}
		}
		else    
		{
			g_Command = STOP_CMD;	
		}
    }
    else if(g_Command == OPEN_CMD)
    {
		viewmode = FILE_VIEW;
	    FileView_Initialize();			
		FileView_OnDraw(g_pScreen);		
    	
    	
		viewmode = FILE_VIEW;	
		bFileSelect = false;		
		g_Command = NEXT_FILE_CMD;	

    }
    else if(g_Command == EXIT_CMD)
    {
    	done = 1;
    }
    
    if(g_Command == STOP_CMD)		
    {
		vstate = STOP_STATUS;			
	    quit_event = 0;
	    break_signal_sent = 0;
	    posCursor = STOP_BUTTON;
		printf("STOP_CMD\n");
		ShowMenu();		
    }
    else
    {
    	printf("### HERE ??? \n");
       	switch_fb1_to_fb0();
	}
}

void ShowResumeMessage()
{
	SDL_Surface *pResumeSurface = IMG_Load(IMG_RESUME);

	if(pResumeSurface != NULL)
	{
		SDL_Rect imgrect;
		imgrect.x = (320-pResumeSurface->w)/2;
		imgrect.y = (240-pResumeSurface->h)/2;
		imgrect.w = pResumeSurface->w;
		imgrect.h = pResumeSurface->h;

		SDL_BlitSurface(pResumeSurface, NULL, g_pScreen, &imgrect);

		//senquack
		SDL_UpdateRect(g_pScreen, 0, 0, 0, 0);
//		SDL_Flip(g_pScreen);

		if(pResumeSurface)	SDL_FreeSurface(pResumeSurface);
		pResumeSurface = NULL;
	}
}

bool GetResumeInfomation(float *pResumePlayTime, char **pszpathname, char **pszfilename)
{
	//senquack - now in same folder as binary for open2x:
//	INI_Open("/usr/gp2x/movie.ini");
	INI_Open("movie.ini");
	*pResumePlayTime = INI_ReadFloat("resume", "time", 0);
	*pszpathname = strdup(INI_ReadText("resume", "path", ""));
	*pszfilename = strdup(INI_ReadText("resume", "file", ""));
	INI_Close();

	return true;
}

SDL_Event *resume_open_event = NULL;

void event_loop(void)
{

    SDL_Event event;      
    SDL_Event open_event; 
	static joydown[32];
	char *resumepathname = NULL;
	char *resumefilename = NULL;

    resume_open_event = &open_event;		// resume 때문에 추가
	memset((char *)&vf, 0x00, sizeof(VideoFile));
    load_count++;

	g_fResumePlayTime = 0.0f;

	if(GetResumeInfomation(&g_fResumePlayTime, &resumepathname, &resumefilename) == true) {
		if(g_fResumePlayTime != 0.0f && resumepathname != NULL && resumefilename != NULL) {
			if(ifile != NULL) free(ifile);
			ifile = (char*)malloc(strlen(resumepathname) + strlen(resumefilename) + 3);
			sprintf(ifile, "%s/%s", resumepathname, resumefilename);

			if(access(ifile, F_OK) == 0) {
				ShowResumeMessage();

				while(!done)
				{
					usleep(1);
					if(SDL_PollEvent(&event) && (event.type == SDL_JOYBUTTONDOWN)) {
						switch(event.jbutton.button) {
							case VK_FB :
								DeleteDirInfo(&infoDir);
								GetDirInfo(&infoDir, resumepathname, FOLDER_AVI_MODE);
								infoView.nStartCount 	= 0;
								infoView.nPosition 		= 0;
								infoView.nStatus 		= 1;
								if(infoDir.nCount > 8)	infoView.nEndCount = 8;
								else					infoView.nEndCount = infoDir.nCount;

								while(1) {
									if(strcmp(infoDir.pList[infoView.nPosition].szName, resumefilename) == 0)	break;

									if(infoView.nPosition >= infoDir.nCount -1)	break;
									infoView.nPosition++;
									if(infoView.nPosition >= infoView.nEndCount) {
										infoView.nStartCount++;
										infoView.nEndCount++;
									}
								}

								g_bResumePlay = true;
								open_event.type = EVENT_MOVIE_PLAY;
								SDL_PushEvent(&open_event);
								done = 1;
								break;
							case VK_FX :
								g_bResumePlay = false;
								FileView_OnDraw(g_pScreen);
								done = 1;
								break;
						}
					}
				}
			}
		}
	}

	if(resumepathname != NULL)	free(resumepathname);		// 할당 받은 것 해제
	if(resumefilename != NULL)	free(resumefilename);		// 할당 받은 것 해제

	done = 0;
    while(!done)
    {
		usleep(1);
		
		if(SDL_PollEvent(&event))			
		{
			switch(event.type)
	        {
				case SDL_JOYBUTTONDOWN :		
					switch(event.jbutton.button)
					{
						case VK_VOL_UP :		
							if( ((g_bHoldLCDoff == false) || (joydown[VK_FL] && joydown[VK_FR])) && (leftVol != 100) )
								volume_change(true);		
							break;
						case VK_VOL_DOWN :		
							if( ((g_bHoldLCDoff == false) || (joydown[VK_FL] && joydown[VK_FR])) && (leftVol != 0) )
								volume_change(false);		
							break;
						case VK_START 	:			
							if(viewmode != FILE_VIEW)
								SaveResumeInfo();
							OnExitClick();				
							break;
						case VK_SELECT:
							resize_mode++;
							break;
					}
			}
			
			
			switch(event.type)
	        {
		        case EVENT_MOVIE_PLAY :			
		        	OnPlay();
		            break;
		        case EVENT_FILE_OPEN :    		
		        	OnFileOpen();
					break;
		        case EVENT_QUIT_PLAY:			
		        	OnQuitPlay(&open_event);
		            break;
		        case EVENT_SUBTITLE_CHANGE:		
		            
		            sub_disp();         
		            break;
		        case EVENT_RESUME_SEEK:
		        	g_fResumePlayTime -= 5;
					force_backward_seek = 1;
		        	if(g_fResumePlayTime < 0)	g_fResumePlayTime = 0;
					OnRelativeSeek(g_fResumePlayTime);
					g_bResumePlay = false;
					g_fResumePlayTime = 0.0f;
		        	break;
		        case SDL_QUIT:					
		            printf("SDL_QUIT signal occur\n");
		            quit_event = 1;
		            done = 1;
		            break;
		        default:
		            break;
	        }
	        
			if(g_bHoldLCDoff == false) {
			switch(event.type)
	        {
				case SDL_JOYBUTTONDOWN :		
					if(viewmode == FILE_VIEW)	
						FileEvnetLoop(&event, &open_event);
					else						
						OnJoystickDown(event.jbutton.button, &open_event);
					break;
				case SDL_JOYBUTTONUP :		
					if(viewmode == FILE_VIEW)	
						FileEvnetLoop(&event, &open_event);
					else
						OnJoystickUp(event.jbutton.button, &open_event);
					break;
		        case SDL_KEYDOWN:				
					if(viewmode == FILE_VIEW)	
						FileEvnetLoop(&event, &open_event);
		        	else						
		        		OnKeyDown(event.key.keysym.sym, &open_event);
		        	break;
		        case SDL_KEYUP:					
					if(viewmode == FILE_VIEW)	
						FileEvnetLoop(&event, &open_event);
					else
						OnKeyUp(event.key.keysym.sym, &open_event);
		        	break;
			}
			} else {
				switch(event.type)
				{
					case SDL_JOYBUTTONDOWN :
					case SDL_JOYBUTTONUP :
						joydown[event.jbutton.button] = event.type == SDL_JOYBUTTONDOWN;
						break;
				}
			}
		}
		
		
	    else if((SDL_GetTicks() > (NowTick + 500)) && (vstate == PLAY_STATUS) && (viewmode == MOVIE_VIEW) && bMenuStatus == true && (bSeekPrevButton == false && bSeekNextButton == false))			
	    {
	    	NowTick = SDL_GetTicks();
			OnHide_SmallNumber();
			OnDraw_SmallNumberText(300,  4, ((int)CurrentPlayTime)/60, ((int)CurrentPlayTime)%60);
			
			OnDraw_Progress(13, 55, (int)(((double)CurrentPlayTime)/((double)TotalPlayTime)*100));
	    }
		
		else if(g_Joystate == SDL_JOYBUTTONDOWN && g_DownTime + 300 < SDL_GetTicks() && viewmode == FILE_VIEW)
		{
			FileView_OnJoystickDown(g_pScreen, g_LastStick, &open_event);
		}
		
		
		else if((bSeekPrevButton || bSeekNextButton || bSeekPrevStick || bSeekNextStick) && (SDL_GetTicks() > g_SeekTime+500) && (SDL_GetTicks() > (NowTick + 100)) && (vstate == PLAY_STATUS) && (viewmode == MOVIE_VIEW))
		{
			int curtime = 0;
	    	NowTick = SDL_GetTicks();
			if(bSeekPrevButton || bSeekPrevStick)
			{
				g_ProgressValue -= 1;
				if(g_ProgressValue < 0)	g_ProgressValue = 0;
			}
			if(bSeekNextButton || bSeekNextStick)
			{
				g_ProgressValue += 1;
				if(g_ProgressValue > 100)	g_ProgressValue = 100;
			}

			curtime = (int)(g_ProgressValue*(TotalPlayTime/100.0));

			if(bMenuStatus) {
				OnDraw_Progress(13, 55, g_ProgressValue);

			OnHide_SmallNumber();
			OnDraw_SmallNumberText(300,  4, ((int)curtime)/60, ((int)curtime)%60);
			} else {
				OnDraw_Progress2(13, 10, g_ProgressValue);

				OnHide_SmallNumber2();
				OnDraw_SmallNumberText(300,  4, ((int)curtime)/60, ((int)curtime)%60);
		
				OnHide_LargeNumber2();
				OnDraw_LargeNumberText(300, 15, ((int)TotalPlayTime)/60, ((int)TotalPlayTime)%60);
			}
		}

	}
}

int SetMode(int argc, char *argv[])
{
/*
	if(argc == 3)
	{
		if(!strcmp(argv[1], "lcd"))				disp_mode = DISPLAY_LCD;
		else if(!strcmp(argv[1], "monitor"))	disp_mode = DISPLAY_MONITOR;
		else if(!strcmp(argv[1], "tv"))			disp_mode = DISPLAY_TV;
		else 									
		{
			printf("NOT argv mode %s\n", argv[1]);
			return -1;
		}
	}
	
	if(argc == 3)
	{
		if(!strcmp(argv[2], "NTSC"))	g_bTVMode = 0;
		else							g_bTVMode = 1;
	}	
*/
    Msgdummy dummymsg, *pdmsg;
    pdmsg = &dummymsg;

    memset((char *) &dummymsg, 0x00, sizeof(Msgdummy));
	MSG(pdmsg) = MMSP2_FB0_TV_LCD_CHECK;
    LEN(pdmsg) = 0;

	 //senquack - SDL_videofd was some sort of retarded global GPH put in their crappy SDL:
	 //		We'll just open /dev/fb0 directly since that is where this ioctl should be going
	int SDL_videofd = open("/dev/fb0", O_RDWR);
	if (SDL_videofd == -1) {
        fprintf(stderr, "Error opening /dev/fb0\n" );
        exit(1);
	}

	if(ioctl(SDL_videofd, FBMMSP2CTRL, pdmsg) == 1) {
		printf("TV Mode");
		disp_mode = DISPLAY_TV;
	} else {
		printf("LCD Mode...\n");
		disp_mode = DISPLAY_LCD;
		  //senquack
		  close(SDL_videofd);
		return 0;
	}

    MSG(pdmsg) = MMSP2_FB0_GET_TV_MODE;
    LEN(pdmsg) = 0;
    g_bTVMode = (ioctl(SDL_videofd, FBMMSP2CTRL, pdmsg) == 1) ? 1 : 0;
	if(g_bTVMode == 0) printf(", NTSC Mode...\n");
	else printf(", PAL Mode...\n");

  //senquack
  close(SDL_videofd);

	return 0;
}

void init_SDL_engine(void)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTTHREAD | SDL_INIT_JOYSTICK))
    {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

	
	if(SDL_NumJoysticks()>0)
	{
		m_pJoy = SDL_JoystickOpen(0);
		if(m_pJoy)
		{
			#if 0
			printf("Opened Joystick 0\n");
			printf("Name: %s\n", SDL_JoystickName(0));
			printf("Number of Axes: %d\n", SDL_JoystickNumAxes(m_pJoy));
			printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(m_pJoy));
			printf("Number of Balls: %d\n", SDL_JoystickNumBalls(m_pJoy));
			#endif
		}
		else
		{
        	fprintf(stderr, "Couldn't open Joystick 0\n");
		}
	}
    
	//senquack -moving this below setvideomode so it actually does get disabled
//	SDL_ShowCursor(SDL_DISABLE);		

	//senquack
    g_pScreen = SDL_SetVideoMode(320, 240, 0, SDL_SWSURFACE | SDL_FULLSCREEN);
//    g_pScreen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF);

    if(g_pScreen == NULL)
    {
        fprintf(stderr, "SDL: could not set video mode - exiting\n");
        exit(1);
    }

	 //	senquack
	SDL_ShowCursor(SDL_DISABLE);		

	INI_Open("./mp2x.ini");

	strcpy(charset, INI_ReadText("subtitle", "lang", ""));

	leftgap = INI_ReadInt("subtitle", "left", 10);
	if(leftgap < 0) leftgap = 0;if(leftgap > 320) leftgap = 320;
	rightgap = INI_ReadInt("subtitle", "right", 10);
	if(rightgap < 0) rightgap = 0;if(rightgap > 320) rightgap = 320;
	topgap = INI_ReadInt("subtitle", "top", 10);
	if(topgap < 0) topgap = 0;if(topgap > 240) topgap = 240;
	bottomgap = INI_ReadInt("subtitle", "bottom", 10);
	if(bottomgap < 0) bottomgap = 0;if(bottomgap > 240) bottomgap = 240;

	minbottom = INI_ReadInt("subtitle", "base", 40);
	if(minbottom < 0) minbottom = 0;
	if(minbottom > (240 - topgap - bottomgap)) minbottom = 240 - topgap - bottomgap;

	font_red = INI_ReadInt("subtitle", "font_red", 0xff) & 0xff;
	font_green = INI_ReadInt("subtitle", "font_green", 0xff) & 0xff;
	font_blue = INI_ReadInt("subtitle", "font_blue", 0xff) & 0xff;

	outline_red = INI_ReadInt("subtitle", "outline_red", 0) & 0xff;
	outline_green = INI_ReadInt("subtitle", "outline_green", 0) & 0xff;
	outline_blue = INI_ReadInt("subtitle", "outline_blue", 0) & 0xff;

	INI_Close();

	//	senquack - now in same folder as binary for open2x
//	INI_Open("/usr/gp2x/common.ini");
	INI_Open("common.ini");

	leftVol = INI_ReadInt("sound", "volume", 70);
	cpuclock = INI_ReadInt("video", "speed", 0);
	if(!charset[0])
		strcpy(charset, INI_ReadText("main", "charset", "UHC//IGNORE"));

	INI_Close();
}
