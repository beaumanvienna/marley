/* Marley Copyright (c) 2020 Marley Development Team 
   https://github.com/beaumanvienna/marley

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include <string>
#include <SDL.h>

#include "../include/marley.h"
#include "../include/statemachine.h"
#include "../include/gui.h"
#include "../include/controller.h"
#include "../include/emu.h"
#include <algorithm>
#include <X11/Xlib.h>
#include <fstream>

#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <dirent.h>
#include <errno.h>


int gState = 0;
int gCurrentGame;
std::vector<string> gGame;
bool gQuit=false;
bool gSetupIsRunning=false;
bool gTextInput=false;
bool gTextInputForGamingFolder = false;
bool gTextInputForFirmwareFolder = false;
string gText;
string gTextForGamingFolder;
string gTextForFirmwareFolder;
bool gControllerConf = false;
int gControllerConfNum=-1;
int confState;
string gConfText;
int gControllerButton[STATE_CONF_MAX];
int xCount,yCount,xValue,yValue;
int gHat[4],gHatValue[4];
int hatIterator;
int gAxis[4];
bool gAxisValue[4];
int axisIterator;
int secondRun;
int secondRunHat;
int secondRunValue;

extern Display* XDisplay;
extern Window Xwindow;
extern int delay_after_shutdown;
extern bool marley_wiimote;
bool checkAxis(int cmd);
bool checkTrigger(int cmd);
void initOpenGL(void);
void setAppIcon(void);
void hide_or_show_cursor_X11(bool hide);
std::ifstream::pos_type filesize(const char* filename);
void resetStatemachine(void)
{
    gState = STATE_OFF;
    gSetupIsRunning = false;
    gTextInput = false;
    gControllerConf = false;
    gControllerConfNum=-1;
}
void create_new_window(void);

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef int                 s32;
typedef unsigned int        u32;
typedef unsigned long int   u64;
typedef struct
{
   s32 session_start;
   s32 session_end;
   u16 session_number;
   u8 total_blocks;
   u8 leadin_blocks;
   u16 first_track;
   u16 last_track;
   u32 unused;
   u32 track_blocks_offset;
} mds_session_struct;

typedef struct
{
   u8 ctl_addr;
   u32 fad_start;
   u32 fad_end;
   u32 file_offset;
   u32 sector_size;
   FILE *fp;
   FILE *sub_fp;
   int file_size;
   int file_id;
   int interleaved_sub;
} track_info_struct;

typedef struct
{
   u32 fad_start;
   u32 fad_end;
   track_info_struct *track;
   int track_num;
} session_info_struct;

typedef struct
{
   int session_num;
   session_info_struct *session;
} disc_info_struct;

#pragma pack(push, 1)
typedef struct
{
   u8 signature[16];
   u8 version[2];
   u16 medium_type;
   u16 session_count;
   u16 unused1[2];
   u16 bca_length;
   u32 unused2[2];
   u32 bca_offset;
   u32 unused3[6];
   u32 disk_struct_offset;
   u32 unused4[3];
   u32 sessions_blocks_offset;
   u32 dpm_blocks_offset;
   u32 enc_key_offset;
} mds_header_struct;
typedef struct
{
   u8 mode;
   u8 subchannel_mode;
   u8 addr_ctl;
   u8 unused1;
   u8 track_num;
   u32 unused2;
   u8 m;
   u8 s;
   u8 f;
   u32 extra_offset;
   u16 sector_size;
   u8 unused3[18];
   u32 start_sector;
   u64 start_offset;
   u8 session;
   u8 unused4[3];
   u32 footer_offset;
   u8 unused5[24];
} mds_track_struct;

typedef struct
{
   u32 filename_offset;
   u32 is_widechar;
   u32 unused1;
   u32 unused2;
} mds_footer_struct;

#pragma pack(pop)
#define MSF_TO_FAD(m,s,f) ((m * 4500) + (s * 75) + f)
static char * wcsdupstr(const wchar_t * path)
{
   char* mbs;
   size_t len = wcstombs(nullptr, path, 0);
   if (len == (size_t) -1) return nullptr;

   mbs = (char*)malloc(len);
   len = wcstombs(mbs, path, len);
   if (len == (size_t) -1)
   {
      free(mbs);
      return nullptr;
   }

   return mbs;
}
static FILE * _wfopen(const wchar_t *wpath, const wchar_t *wmode)
{
   FILE * fd;
   char * path;
   char * mode;

   path = wcsdupstr(wpath);
   if (path == nullptr) return nullptr;

   mode = wcsdupstr(wmode);
   if (mode == nullptr)
   {
      free(path);
      return nullptr;
   }

   fd = fopen(path, mode);

   free(path);
   free(mode);

   return fd;
}

int LoadMDSTracks(const char* mds_filename, FILE* mds_file, mds_session_struct* mds_session, session_info_struct* session)
{
   int i;
   int track_num=0;
   u32 fad_end = 0;

   session->track = (track_info_struct*)malloc(sizeof(track_info_struct) * mds_session->last_track);
   if (session->track == nullptr)
   {
      printf("error 7 memory allocation \n");
      return -1;
   }
	memset(session->track, 0, sizeof(track_info_struct) * mds_session->last_track);

   for (i = 0; i < mds_session->total_blocks; i++)
   {
      mds_track_struct track;
      FILE *fp=nullptr;
      int file_size = 0;

      fseek(mds_file, mds_session->track_blocks_offset + i * sizeof(mds_track_struct), SEEK_SET);
      if (fread(&track, 1, sizeof(mds_track_struct), mds_file) != sizeof(mds_track_struct))
      {
         printf("error 8 could not read mds file \n");
         free(session->track);
         return -1;
      }

      if (track.track_num == 0xA2)
         fad_end = MSF_TO_FAD(track.m, track.s, track.f);
      if (!track.extra_offset)
         continue;

      if (track.footer_offset)
      {
         mds_footer_struct footer;
         int found_dupe=0;
         int j;

         // Make sure the file was not opened already
         for (j = 0; j < track_num; j++)
         {
            if (track.footer_offset == session->track[j].file_id)
            {
               found_dupe = 1;
               break;
            }
         }

         if (found_dupe)
         {
            fp = session->track[j].fp;
            file_size = session->track[j].file_size;
         }
         else
         {
            fseek(mds_file, track.footer_offset, SEEK_SET);
            if (fread(&footer, 1, sizeof(mds_footer_struct), mds_file) != sizeof(mds_footer_struct))
            {
               printf("error 9 could not read mds file \n");
               free(session->track);
               return -1;
            }

            fseek(mds_file, footer.filename_offset, SEEK_SET);
            if (footer.is_widechar)
            {
               wchar_t filename[512];
               wchar_t img_filename[512];
               memset(img_filename, 0, 512 * sizeof(wchar_t));
               
               if (fwscanf(mds_file, L"%512c", img_filename) != 1)
               {
                  printf("error 9 could not read img file \n");
                  free(session->track);
                  return -1;
               }

               if (wcsncmp(img_filename, L"*.", 2) == 0)
               {
                  wchar_t *ext;
                  swprintf(filename, sizeof(filename)/sizeof(wchar_t), L"%S", mds_filename);
                  ext = wcsrchr(filename, '.');
                  wcscpy(ext, img_filename+1);
               }
               else
                  wcscpy(filename, img_filename);
               fp = _wfopen(filename, L"rb");
            }
            else
            {
               char filename[512];
               char img_filename[512];
               memset(img_filename, 0, 512);

               if (fscanf(mds_file, "%512c", img_filename) != 1)
               {
                  printf("error 10 could not read img file \n");
                  free(session->track);
                  return -1;
               }

               if (strncmp(img_filename, "*.", 2) == 0)
               {
                  char *ext;
                  size_t mds_filename_len = strlen(mds_filename);
                  if (mds_filename_len >= 512)
                  {
                     printf("error 11 mds file length too big \n");
                     free(session->track);
                     return -1;
                  }
                  strcpy(filename, mds_filename);
                  ext = strrchr(filename, '.');
                  strcpy(ext, img_filename+1);
               }
               else
                  strcpy(filename, img_filename);
               fp = fopen(filename, "rb");
            }

            if (fp == nullptr)
            {
               printf("error 12 could not open file \n");
               free(session->track);
               return -1;
            }

            fseek(fp, 0, SEEK_END);
            file_size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
         }
      }

      session->track[track_num].ctl_addr = (((track.addr_ctl << 4) | (track.addr_ctl >> 4)) & 0xFF);
      session->track[track_num].fad_start = track.start_sector+150;
      if (track_num > 0)
         session->track[track_num-1].fad_end = session->track[track_num].fad_start;
      session->track[track_num].file_offset = track.start_offset;
      session->track[track_num].sector_size = track.sector_size;
      session->track[track_num].fp = fp;
      session->track[track_num].file_size = file_size;
      session->track[track_num].file_id = track.footer_offset;
      session->track[track_num].interleaved_sub = track.subchannel_mode != 0 ? 1 : 0;

      track_num++;
   }

   session->track[track_num-1].fad_end = fad_end;
   session->fad_start = session->track[0].fad_start;
   session->fad_end = fad_end;
   session->track_num = track_num;
   return 0;
}

int LoadMDS(const char *mds_filename, FILE *mds_file)
{
   s32 i;
   mds_header_struct header;
   disc_info_struct disc;

   fseek(mds_file, 0, SEEK_SET);

   if (fread((void *)&header, 1, sizeof(mds_header_struct), mds_file) != sizeof(mds_header_struct))
   {
      printf("error 1 mds file (bad size)\n");
   }
   else if (memcmp(&header.signature,  "MEDIA DESCRIPTOR", sizeof(header.signature)))
   {
      printf("error 2 mds file (bad mds descriptor)\n");
   }
   else if (header.version[0] > 1)
   {
      printf("error 3 mds file (bad version)\n");
   }

   if (header.medium_type & 0x10)
   {
      // DVDs are not supported
      printf("error 4 mds file (bad medium type)\n");
   }

   disc.session_num = header.session_count;
   disc.session = (session_info_struct*)malloc(sizeof(session_info_struct) * disc.session_num);
   if (disc.session == nullptr)
   {
      printf("error 5 mds file (bad session number)\n");
   }

   for (i = 0; i < header.session_count; i++)
   {
      mds_session_struct session;

      fseek(mds_file, header.sessions_blocks_offset + i * sizeof(mds_session_struct), SEEK_SET);
      if (fread(&session, 1, sizeof(mds_session_struct), mds_file) != sizeof(mds_session_struct))
      {
         free(disc.session);
         printf("error 6 mds file \n");
      }

      LoadMDSTracks(mds_filename, mds_file, &session, &disc.session[i]);
   }

   fclose(mds_file);

   return 0;
}

bool exists(const char *fileName);
int mdf2iso_main (int argc, char **argv);
void create_cue_file(string filename)
{
    string ext = filename.substr(filename.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(),
      [](unsigned char c){ return std::tolower(c); });
    string bin_filename = filename.substr(0,filename.find_last_of(".")) + ".bin";
    string cue_filename = filename.substr(0,filename.find_last_of(".")) + ".cue";
    if (ext.find("bin") != string::npos) 
    {
        std::ofstream cue_file;

        cue_file.open(cue_filename.c_str(), std::ios_base::trunc); 
        if(cue_file) 
        {
            cue_file << "FILE \"";
            if (filename.find_last_of("/") != string::npos)
            {
                cue_file << filename.substr(filename.find_last_of("/")+1);
            }
            else
            {
                cue_file << filename;
            }
            cue_file << "\" BINARY\n";
            cue_file << "  TRACK 01 MODE2/2352\n";
            cue_file << "    INDEX 01 00:00:00";
            cue_file.close();
            // replace bin file
        }
        else
        {
            printf("Could not create cue file for \n",filename.c_str());
        }
    }
    else if ((ext.find("mdf") != string::npos) && (!exists(bin_filename.c_str())))
    {
        string bin_filename_no_path = bin_filename;
        if (bin_filename.find("/") != string::npos)
        {
            bin_filename_no_path = bin_filename.substr(bin_filename.find_last_of("/") + 1);
        }
        char *argv[4]; 
		char arg1[10] = "mdf2iso"; 
		char arg2[10] = "--cue";
		char arg3[1024];
        char arg4[1024];
        strcpy(arg3, filename.c_str());  
        strcpy(arg4, bin_filename_no_path.c_str());

        argv[0] = arg1;
        argv[1] = arg2;
        argv[2] = arg3;
        argv[3] = arg4;
        
        mdf2iso_main (4, argv);
    }
    gGame[gCurrentGame] = cue_filename;
}

#define BUFSIZE 1024
enum emulator_target
{
	unknown,
	mednafen,
	dolphin,
	mupen64plus,
	ppsspp,
	pcsx2
};
emulator_target getEmulatorTarget(string filename)
{
    string cmd = "file -b \"" + filename + "\"";
    string file_type;
    char buf[BUFSIZE];
    FILE *fp;
    bool ok = false;
    emulator_target emu = unknown;
	
    if ((fp = popen(cmd.c_str(), "r")) == nullptr) 
    {
        printf("Error opening pipe for command %s\n",cmd.c_str());
    }
    else
    {
        if (fgets(buf, BUFSIZE, fp) != nullptr) 
        {
            file_type = buf;
            ok = true;
        }
        
        if(pclose(fp))  
        {
			printf("Command '%s' not found or exited with error status\n",cmd.c_str());
			ok = false;
		}
    }
    
    if (ok)
    {
		
		std::transform(file_type.begin(), file_type.end(), file_type.begin(),
			[](unsigned char c){ return std::tolower(c); });
		
		if ((file_type.find("nintendo wii") != string::npos) || (file_type.find("nintendo gamecube") != string::npos))
		{
			emu = dolphin; 
			printf("dolphin ");
		}
		else if (file_type.find("game boy") != string::npos)
		{
			emu = mednafen;
			printf("mednafen ");
		}
		else if ((file_type.find("sega mega drive") != string::npos) || (file_type.find("genesis") != string::npos))
		{
			emu = mednafen;
			printf("mednafen ");
		}
		else if (file_type.find("sega saturn") != string::npos)
		{
            create_cue_file(filename);
			emu = mednafen;
			printf("mednafen ");
		}
		else if (file_type.find("nes rom") != string::npos)
		{
			emu = mednafen;
			printf("mednafen ");
		}
		else if (file_type.find("nintendo 64") != string::npos)
		{
			emu = mupen64plus;
			printf("mupen64plus ");
		}
		else
		{
			long fsize = filesize(filename.c_str());
			if (fsize < 52428800) // less than 50MB must be either mednafen or mupen64plus
			{
				string ext = filename.substr(filename.find_last_of(".") + 1);
				if (ext.find("64") != string::npos) 
				{
					emu = mupen64plus;
				}
				else
				{
					emu = mednafen;
				}
			}
			else
			{
				if (file_type.find("iso 9660 cd-rom filesystem data") != string::npos)
				{
					emu = ppsspp;
					printf("ppsspp ");
				}
				else 
				{
					emu = pcsx2;
					printf("pcsx2 ");
				}
			}
		}
	}

    return emu;
}

void shutdown_computer(void)
{
    string cmd = "shutdown now";
    FILE *fp;
    
    if ((fp = popen(cmd.c_str(), "r")) == nullptr) 
    {
        printf("Error opening pipe for command %s\n",cmd.c_str());
    }
}

void launch_emulator(void)
{
	if (gGame[gCurrentGame] != "")
	{
		int argc;
		char *argv[10]; 
		
		char arg1[1024]; 
		char arg2[1024];
		char arg3[1024]; 
		char arg4[1024]; 
		char arg5[1024]; 
		char arg6[1024]; 
		char arg7[1024]; 
		char arg8[1024]; 
		char arg9[1024]; 
		char arg10[1024]; 
		
		int n;
		string str;
		
		window_flags=SDL_GetWindowFlags(gWindow);
		if (!(window_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) && !(window_flags & SDL_WINDOW_FULLSCREEN))
		{
			SDL_GetWindowSize(gWindow,&window_width,&window_height);
			SDL_GetWindowPosition(gWindow,&window_x,&window_y);
		}
		
		argc = 2;
		auto emulatorTarget = getEmulatorTarget(gGame[gCurrentGame]);
		switch((int)emulatorTarget)
		{
#ifdef PCSX2
			case pcsx2:
				str = "pcsx2";
				n = str.length(); 
				strcpy(arg1, str.c_str()); 
				
				str = "--spu2=";
				str += gBaseDir;
				str += "PCSX2/libspu2x-2.0.0.so";
				n = str.length(); 
				strcpy(arg2, str.c_str()); 
				
				str = "--cdvd=";
				str += gBaseDir;
				str += "PCSX2/libCDVDnull.so";
				n = str.length(); 
				strcpy(arg3, str.c_str()); 

				str = "--usb=";
				str += gBaseDir;
				str += "PCSX2/libUSBnull-0.7.0.so";
				n = str.length(); 
				strcpy(arg4, str.c_str()); 

				str = "--fw=";
				str += gBaseDir;
				str += "PCSX2/libFWnull-0.7.0.so";
				n = str.length(); 
				strcpy(arg5, str.c_str()); 

				str = "--dev9=";
				str += gBaseDir;
				str += "PCSX2/libdev9null-0.5.0.so";
				n = str.length(); 
				strcpy(arg6, str.c_str()); 
				
				str = "--fullboot";
				n = str.length(); 
				strcpy(arg7, str.c_str()); 

				str = gGame[gCurrentGame];
				n = str.length(); 
				strcpy(arg8, str.c_str());
				
				str = "--nogui";
				n = str.length(); 
				strcpy(arg9, str.c_str());

				argv[0] = arg1;
				argv[1] = arg2;
				argv[2] = arg3;
				argv[3] = arg4;
				argv[4] = arg5;
				argv[5] = arg6;
				argv[6] = arg7;
				argv[7] = arg8;
				argv[8] = arg9;

				argc = 9;
				
				freeTextures();
				SDL_DestroyRenderer( gRenderer );
				SDL_DestroyWindow(gWindow);
				SDL_QuitSubSystem(SDL_INIT_VIDEO);
				create_new_window();
				pcsx2_main(argc,argv);
				restoreSDL();
				break;
#endif


#ifdef MUPEN64PLUS
			case mupen64plus:
				
				str = "mupen64plus";
				n = str.length(); 
				strcpy(arg1, str.c_str()); 
				
				n = gGame[gCurrentGame].length(); 
				strcpy(arg2, gGame[gCurrentGame].c_str()); 
				
				argv[0] = arg1;
				argv[1] = arg2;
				printf("arg1: %s arg2: %s \n",arg1,arg2);
				mupen64plus_main(argc,argv);
				restoreSDL();
				break;
#endif

#ifdef PPSSPP
		
			case ppsspp:
			
				str = "ppsspp";
				n = str.length(); 
				strcpy(arg1, str.c_str()); 
				
				n = gGame[gCurrentGame].length(); 
				strcpy(arg2, gGame[gCurrentGame].c_str()); 
				
				argv[0] = arg1;
				argv[1] = arg2;
				printf("arg1: %s arg2: %s \n",arg1,arg2);
				ppsspp_main(argc,argv);
				
				restoreSDL();
				break;
#endif

#ifdef DOLPHIN
		
			case dolphin:
				str = "dolphin-emu";
				n = str.length(); 
				strcpy(arg1, str.c_str()); 
				
				n = gGame[gCurrentGame].length(); 
				strcpy(arg2, gGame[gCurrentGame].c_str()); 
				
				argv[0] = arg1;
				argv[1] = arg2;
				printf("arg1: %s arg2: %s \n",arg1,arg2);
				marley_wiimote = false;
				dolphin_main(argc,argv);
				marley_wiimote = true;
				delay_after_shutdown = 10;
				restoreSDL();
				break;
#endif
		
		
#ifdef MEDNAFEN
			case mednafen:
				str = "mednafen";
				n = str.length(); 
				strcpy(arg1, str.c_str()); 
				
				n = gGame[gCurrentGame].length(); 
				strcpy(arg2, gGame[gCurrentGame].c_str()); 
				
				argv[0] = arg1;
				argv[1] = arg2;
				printf("arg1: %s arg2: %s \n",arg1,arg2);
				mednafen_main(argc,argv);
				restoreSDL();
				break;
#endif
			default:
				(void) 0;
				break;
		}
	}
}

void statemachine(int cmd)
{
    string execute;
    bool emuReturn = false;
    
    if (!gControllerConf)
    {
        switch (cmd)
        {
            case SDL_CONTROLLER_BUTTON_GUIDE:
                if (gState == STATE_OFF)
                {
                    gQuit=true;
                }
                else
                {
                    resetStatemachine();
                }
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: 
            case SDLK_DOWN:
            case SDLK_RIGHT:
                switch (gState)
                {
                    case STATE_ZERO:
                        if (gNumDesignatedControllers)
                        {
                            if (gGamesFound)
                            {
                                gState=STATE_PLAY;
                            }
                            else
                            {
                                gState=STATE_SETUP;
                            }
                        }
                        else
                        {
                            gState=STATE_OFF;
                        }
                        break;
                    case STATE_PLAY:
                        gState=STATE_SETUP;
                        break;
                    case STATE_SETUP:
                        gState=STATE_OFF;
                        break;
                    case STATE_SHUTDOWN:
                    case STATE_OFF:
                        if (gGamesFound)
                        {
                            gState=STATE_LAUNCH;
                        }
                        else
                        {
                            gState=STATE_SETUP;
                        }
                        break;
                     case STATE_CONF0:
                        if (gDesignatedControllers[1].numberOfDevices != 0)
                        {
                            gState=STATE_CONF1;
                        } 
                        else 
                        {
                            gState=STATE_FLR_GAMES;
                        }
                        break;
                    case STATE_CONF1:
                        gState=STATE_FLR_GAMES;
                        break;
                    case STATE_FLR_GAMES:
                        gState=STATE_FLR_FW;
                        break;
                    case STATE_LAUNCH:
                        if (gCurrentGame == (gGame.size()-1))
                        {
                            gState=STATE_PLAY;
                        }
                        else
                        {
                            gCurrentGame++;
                        }
                        break;
                    default:
                        (void) 0;
                        break;
                }
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_UP:
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            case SDLK_UP:
            case SDLK_LEFT:
                switch (gState)
                {
                    case STATE_ZERO:
                        if (gNumDesignatedControllers)
                        {
                            if (gGamesFound)
                            {
                                gState=STATE_LAUNCH;
                            }
                            else
                            {
                                gState=STATE_OFF;
                            }
                        }
                        else
                        {
                            gState=STATE_OFF;
                        }
                        break;
                    case STATE_PLAY:
                        gState=STATE_LAUNCH;
                        break;
                    case STATE_SETUP:
                        if (gGamesFound)
                        {
                            gState=STATE_PLAY;
                        }
                        else
                        {
                            gState=STATE_OFF;
                        }
                        break;
                    case STATE_OFF:
                        if (gNumDesignatedControllers)
                        {
                            gState=STATE_SETUP;
                        }
                        break;
                    case STATE_SHUTDOWN:
                        gState=STATE_OFF;
                        break;
                     case STATE_CONF0:
                        gState=STATE_SETUP;
                        gSetupIsRunning=false;
                        break;
                    case STATE_CONF1:
                        if (gDesignatedControllers[0].numberOfDevices != 0)
                        {
                            gState=STATE_CONF0;
                        } 
                        else 
                        {
                            gSetupIsRunning=false;
                            gState=STATE_SETUP;
                        }
                        break;
                    case STATE_LAUNCH:
                    
                        if (gCurrentGame == 0)
                        {
                            gState=STATE_OFF;
                        }
                        else
                        {
                            gCurrentGame--;
                        }
                        break;
                    case STATE_FLR_GAMES:
                        if (gDesignatedControllers[1].numberOfDevices != 0)
                        {
                            gState=STATE_CONF1;
                        } 
                        else if (gDesignatedControllers[0].numberOfDevices != 0)
                        {
                            gState=STATE_CONF0;
                        }
                        else 
                        {
                            gSetupIsRunning=false;
                            gState=STATE_SETUP;
                        }
                        break;
                    case STATE_FLR_FW:
                            gState=STATE_FLR_GAMES;
                        break;
                    default:
                        (void) 0;
                        break;
                }
                break;
            case SDL_CONTROLLER_BUTTON_B:
                if (gState == STATE_OFF) gState = STATE_SHUTDOWN;
                break;
            case SDL_CONTROLLER_BUTTON_A:
                switch (gState)
                {
                    case STATE_ZERO:
                        
                        break;
                    case STATE_PLAY:
                        gState=STATE_LAUNCH;
                        break;
                    case STATE_SETUP:
                        if (gDesignatedControllers[0].numberOfDevices != 0)
                        {
                            gState=STATE_CONF0;
                        } 
                        else if (gDesignatedControllers[1].numberOfDevices != 0)
                        {
                            gState=STATE_CONF1;
                        }
                        gSetupIsRunning=true;
                        break;
                    case STATE_OFF:
                        gQuit=true;
                        break;
                    case STATE_SHUTDOWN:
                        gQuit=true;
                        shutdown_computer();
                        break;
                     case STATE_CONF0:
                     case STATE_CONF1:
                        for (int i=0;i<STATE_CONF_MAX;i++)
                        {
                            gControllerButton[i]=STATE_CONF_SKIP_ITEM;
                        }
                        
                        for (int i = 0; i < 4;i++)
                        {
                            gHat[i] = -1;
                            gHatValue[i] = -1;
                            gAxis[i] = -1;
                            gAxisValue[i] = false;
                        }
                        hatIterator = 0;
                        axisIterator = 0;
                        secondRun = -1;
                        secondRunHat = -1;
                        secondRunValue = -1;
                        
                        gControllerConf=true;
                        if (gState==STATE_CONF0)
                        {
                            gControllerConfNum=0;
                        }
                        else
                        {
                            gControllerConfNum=1;
                        }
                        confState = STATE_CONF_BUTTON_DPAD_UP;
                        gConfText = "press dpad up";
                        break;
                    case STATE_FLR_GAMES:
                        if (!gTextInputForGamingFolder)
                        {
                            gTextInputForGamingFolder=true;
                            gTextInput = true;
                            
                            const char *homedir;
                            string slash;
        
                            if ((homedir = getenv("HOME")) != nullptr) 
                            {
                                gText = homedir;
                                
                                slash = gText.substr(gText.length()-1,1);
                                if (slash != "/")
                                {
                                    gText += "/";
                                }
                            }
                            else
                            {
                                gText = "";
                            }
                        }
                        else
                        {
                            if (gTextInput) // input exit with RETURN
                            {
                                if (setPathToGames(gText))
                                {
                                    
                                    string setting = "search_dir_games=";
                                    setting += gText;
                                    addSettingToConfigFile(setting);
                                    
                                    gTextForGamingFolder=gText;
                                    buildGameList();
                                    checkFirmwarePSX();
                                }
                            }
                            else // input exit with ESC
                            {
                                gTextForGamingFolder=gText;
                            }
                            gTextInputForGamingFolder=false;
                            gTextInput = false;
                        }
                        break;
                    case STATE_FLR_FW:
                        if (!gTextInputForFirmwareFolder)
                        {
                            gTextInputForFirmwareFolder=true;
                            gTextInput = true;
                            
                            const char *homedir;
                            string slash;
        
                            if ((homedir = getenv("HOME")) != nullptr) 
                            {
                                gText = homedir;
                                
                                slash = gText.substr(gText.length()-1,1);
                                if (slash != "/")
                                {
                                    gText += "/";
                                }
                            }
                            else
                            {
                                gText = "";
                            }
                        }
                        else
                        {
                            if (gTextInput) // input exit with RETURN
                            {
                                if (setPathToFirmware(gText))
                                {
                                    
                                    string setting = "search_dir_firmware_PSX=";
                                    setting += gText;
                                    addSettingToConfigFile(setting);
                                    
                                    gTextForFirmwareFolder=gText;
                                    checkFirmwarePSX();
                                }
                            }
                            else // input exit with ESC
                            {
                                gTextForFirmwareFolder=gText;
                            }
                            gTextInputForFirmwareFolder=false;
                            gTextInput = false;
                        }
                        break;
                    case STATE_LAUNCH:
						launch_emulator();
                        break;
                    default:
                        (void) 0;
                        break;
                }
                break;
            default:
                (void) 0;
                break;
        }
    }
    else // controller conf
    {
        
    }
}


void statemachineConf(int cmd)
{
    if ((cmd==STATE_CONF_SKIP_ITEM) && (confState > STATE_CONF_BUTTON_RIGHTSHOULDER)) statemachineConfAxis(STATE_CONF_SKIP_ITEM,false);
    if ( (gControllerConf) && (confState <= STATE_CONF_BUTTON_RIGHTSHOULDER) )
    {
        if ((gActiveController == gControllerConfNum) || (cmd==STATE_CONF_SKIP_ITEM))
        {
            switch (confState)
            {
                case STATE_CONF_BUTTON_DPAD_UP:
                    if (secondRun == -1)
                    {
                        gConfText = "press dpad up";
                        secondRun = cmd;
                    }
                    else if (secondRun == cmd)
                    {
                        gControllerButton[confState]=cmd;
                        confState = STATE_CONF_BUTTON_DPAD_DOWN;
                        gConfText = "press dpad down";
                        secondRun = -1;
                    }
                    break;
                case STATE_CONF_BUTTON_DPAD_DOWN:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_DPAD_LEFT;
                    gConfText = "press dpad left";
                    break;
                case STATE_CONF_BUTTON_DPAD_LEFT:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_DPAD_RIGHT;
                    gConfText = "press dpad right";
                    break;
                case STATE_CONF_BUTTON_DPAD_RIGHT:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_A;
                    gConfText = "press button A (lower)";
                    break;
                case STATE_CONF_BUTTON_A:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_B;
                    gConfText = "press button B (right)";
                    break;
                case STATE_CONF_BUTTON_B:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_X;
                    gConfText = "press button X (left)";
                    break;
                case STATE_CONF_BUTTON_X:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_Y;
                    gConfText = "press button Y (upper)";
                    break;
                case STATE_CONF_BUTTON_Y:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_LEFTSTICK;
                    gConfText = "press left stick button";
                    break;
                case STATE_CONF_BUTTON_LEFTSTICK:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_RIGHTSTICK;
                    gConfText = "press right stick button";
                    break;
                case STATE_CONF_BUTTON_RIGHTSTICK:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_LEFTSHOULDER;
                    gConfText = "press left front shoulder";
                    break;
                case STATE_CONF_BUTTON_LEFTSHOULDER:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_RIGHTSHOULDER;
                    gConfText = "press right front shoulder";
                    break;
                case STATE_CONF_BUTTON_RIGHTSHOULDER:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_BACK;
                    gConfText = "press select/back button";
                    break;
                case STATE_CONF_BUTTON_BACK:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_START;
                    gConfText = "press start button";
                    break;
                case STATE_CONF_BUTTON_START:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_BUTTON_GUIDE;
                    gConfText = "press guide button";
                    break;
                case STATE_CONF_BUTTON_GUIDE:
                    gControllerButton[confState]=cmd;
                    confState = STATE_CONF_AXIS_LEFTSTICK_X;
                    gConfText = "twirl left stick";
                    xCount=0;
                    yCount=0;
                    xValue=-1;
                    yValue=-1;
                    break;
                default:
                    (void) 0;
                    break;
            }
        }
    }
}

void statemachineConfAxis(int cmd, bool negative)
{
    if ( (gControllerConf) && (confState >= STATE_CONF_AXIS_LEFTSTICK_X) )
    {
        if ((gActiveController == gControllerConfNum)  || (cmd==STATE_CONF_SKIP_ITEM))
        {
            switch (confState)
            {
                case STATE_CONF_AXIS_LEFTSTICK_X:
                case STATE_CONF_AXIS_LEFTSTICK_Y:
                    if (checkAxis(cmd))
                    {
                        printf("***** x axis: %i, y axis: %i\n",xValue,yValue);
                        xCount=0;
                        yCount=0;
                        xValue=-1;
                        yValue=-1;

                        confState = STATE_CONF_AXIS_RIGHTSTICK_X;
                        gConfText = "twirl right stick";
                    }
                    break;
                case STATE_CONF_AXIS_RIGHTSTICK_X:
                case STATE_CONF_AXIS_RIGHTSTICK_Y:
                    if (cmd == STATE_CONF_SKIP_ITEM)
                    {
                        xCount=0;
                        yCount=0;
                        xValue=-1;
                        yValue=-1;
                        
                        confState = STATE_CONF_AXIS_LEFTTRIGGER;
                        gConfText = "press left rear shoulder";
                    }
                    else if ( (cmd != gControllerButton[STATE_CONF_AXIS_LEFTSTICK_X]) &&\
                            (cmd != gControllerButton[STATE_CONF_AXIS_LEFTSTICK_Y]))
                    {
                        
                        if (checkAxis(cmd))
                        {
                            printf("***** x axis: %i, y axis: %i\n",xValue,yValue);
                            xCount=0;
                            yCount=0;
                            xValue=-1;
                            yValue=-1;
                            
                            confState = STATE_CONF_AXIS_LEFTTRIGGER;
                            gConfText = "press left rear shoulder";
                        }
                    }
                    break;
                case STATE_CONF_AXIS_LEFTTRIGGER:
                    if (cmd == STATE_CONF_SKIP_ITEM)
                    {
                        xCount=0;
                        xValue=-1;
                          
                        confState = STATE_CONF_AXIS_RIGHTTRIGGER;
                        gConfText = "press right rear shoulder";
                    }
                    else if ( (cmd != gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_X]) &&\
                            (cmd != gControllerButton[STATE_CONF_AXIS_RIGHTSTICK_Y]))
                    {
                        if (checkTrigger(cmd))
                        {
                            printf("***** trigger: %i\n",xValue);
                            xCount=0;
                            xValue=-1;
                            
                            confState = STATE_CONF_AXIS_RIGHTTRIGGER;
                            gConfText = "press right rear shoulder";
                        }
                    }
                    
                    break;
                case STATE_CONF_AXIS_RIGHTTRIGGER:
                    if (cmd == STATE_CONF_SKIP_ITEM)
                    {
                        xCount=0;
                        xValue=-1;
                          
                        gConfText = "configuration done";
                        setMapping();
                    }
                    else if (cmd != gControllerButton[STATE_CONF_AXIS_LEFTTRIGGER]) 
                    {
                        if (checkTrigger(cmd))
                        {
                            printf("***** trigger: %i\n",xValue);
                            xCount=0;
                            xValue=-1;
                            gConfText = "configuration done";
                            setMapping();
                        }
                    }
                    break;
                default:
                    (void) 0;
                    break;
            }
        }
    } else if ( (gControllerConf) && (confState <= STATE_CONF_BUTTON_DPAD_RIGHT) )
    {
        if ((gActiveController == gControllerConfNum)  || (cmd==STATE_CONF_SKIP_ITEM))
        {
            gAxis[axisIterator] = cmd;
            gAxisValue[axisIterator] = negative;
            switch (confState)
            {
                case STATE_CONF_BUTTON_DPAD_UP:
                    gConfText = "press dpad down";
                    break;
                case STATE_CONF_BUTTON_DPAD_DOWN:
                    gConfText = "press dpad left";
                    break;
                case STATE_CONF_BUTTON_DPAD_LEFT:
                    gConfText = "press dpad right";
                    break;
                case STATE_CONF_BUTTON_DPAD_RIGHT:
                    gConfText = "press button A (lower)";
                    break;
                default:
                    (void) 0;
                    break;
            }
            confState++;
            axisIterator++;
        }
    }
}

bool checkAxis(int cmd)
{
    if (cmd==STATE_CONF_SKIP_ITEM) return true;
    
    bool ok = false;
    
    if ( (xCount > 20) && (yCount>20) )
    {
        gControllerButton[confState]=xValue;
        gControllerButton[confState+1]=yValue;
        ok = true;
    }
    
    if ( (xValue!=-1) && (yValue!=-1) )
    {
        if (xValue == cmd) xCount++;
        if (yValue == cmd) yCount++;
    }
    
    if ( (xValue!=-1) && (yValue==-1) )
    {
        if (xValue > cmd)
        {
            yValue = xValue;
            xValue = cmd;
        }
        else if (xValue != cmd)
        {
            yValue = cmd;
        }
    }
    
    if ( (xValue==-1) && (yValue==-1) )
    {
        xValue=cmd;
    }   
    
    printf("axis confState: %i, cmd: %i, xValue: %i, yValue: %i, xCount: %i, yCount: %i \n",confState,cmd,xValue,yValue,xCount,yCount); 
    return ok;
}

bool checkTrigger(int cmd)
{
    if (cmd==STATE_CONF_SKIP_ITEM) return true;
    bool ok = false;
    
    if ( (xCount > 20)  )
    {
        gControllerButton[confState]=xValue;
        ok = true;
    }
    
    if (xValue!=-1)
    {
        if (xValue == cmd) xCount++;
    }
    
    if (xValue==-1)
    {
        xValue=cmd;
    }   
    
    printf("axis confState: %i, cmd: %i, xValue: %i, xCount: %i\n",confState,cmd,xValue,xCount); 
    return ok;
}


void statemachineConfHat(int hat, int value)
{    
    printf("ConfHat hat: %i, value: %i\n",hat,value);
    
    gHat[hatIterator] = hat;
    gHatValue[hatIterator] = value;
    
    if (gActiveController == gControllerConfNum)
    {
        
        switch (confState)
        {
            case STATE_CONF_BUTTON_DPAD_UP:            
                if ( (secondRunHat == -1) && (secondRunValue == -1) )
                {
                    gConfText = "press dpad up again";
                    secondRunHat = hat;
                    secondRunValue = value;
                }
                else if ( (secondRunHat == hat) && (secondRunValue == value) )
                {
                    hatIterator++;
                    confState = STATE_CONF_BUTTON_DPAD_DOWN;
                    gConfText = "press dpad down";
                    secondRunHat = -1;
                    secondRunValue = -1;
                }
                break;
            case STATE_CONF_BUTTON_DPAD_DOWN:
                confState = STATE_CONF_BUTTON_DPAD_LEFT;
                gConfText = "press dpad left";
                hatIterator++;
                break;
            case STATE_CONF_BUTTON_DPAD_LEFT: 
                confState = STATE_CONF_BUTTON_DPAD_RIGHT;
                gConfText = "press dpad right";
                hatIterator++;
                break;
            case STATE_CONF_BUTTON_DPAD_RIGHT: 
                confState = STATE_CONF_BUTTON_A;
                gConfText = "press button A (lower)";
                hatIterator++;
                break;
            default:
                (void) 0;
                break;
        }
    }
}
