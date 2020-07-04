/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-core - osal/files_unix.c                                  *
 *   Mupen64Plus homepage: https://mupen64plus.org/                        *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       * 
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
                       
/* This file contains the definitions for the unix-specific file handling
 * functions
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "api/callbacks.h"
#include "api/m64p_types.h"
#include "files.h"

// definitions for system directories to search when looking for shared data files
#if defined(SHAREDIR)
  #define XSTR(S) STR(S) // this wacky preprocessor thing is necessary to generate a quote-enclosed
  #define STR(S) #S      // copy of the SHAREDIR macro, which is defined by the makefile via gcc -DSHAREDIR="..."
  static const int   datasearchdirs = 4;
  static const char *datasearchpath[4] = { XSTR(SHAREDIR), "/usr/local/share/mupen64plus",  "/usr/share/mupen64plus", "./" };
  #undef STR
  #undef XSTR
#else
  static const int   datasearchdirs = 3;
  static const char *datasearchpath[3] = { "/usr/local/share/mupen64plus",  "/usr/share/mupen64plus", "./" };
#endif

// local functions
void outputGlideIni(FILE * fp);
char * globalBaseDirectory;
static int get_xdg_dir(char *destpath, const char *envvar, const char *subdir)
{
    
    struct stat fileinfo;
    const char *envpath  = getenv(envvar);
    char glideIni[PATH_MAX];
    
    if (globalBaseDirectory) return 0;
    
    globalBaseDirectory=getenv(envvar);

    // error if this environment variable doesn't return a good string
    if (envpath == NULL || strlen(envpath) < 1)
        return 1;

    // error if path returned by the environemnt variable isn't a valid path to a directory
    if (stat(envpath, &fileinfo) != 0 || !S_ISDIR(fileinfo.st_mode))
        return 2;

    // append the given sub-directory to the path given by the environment variable
    strcpy(destpath, envpath);
    if (destpath[strlen(destpath)-1] != '/')
        strcat(destpath, "/");
    strcat(destpath, subdir);

    // try to create the resulting directory tree, or return successfully if it already exists
    if (osal_mkdirp(destpath, 0700) != 0)
    {
        DebugMessage(M64MSG_ERROR, "Couldn't create directory: %s", destpath);
        return 3;
    }
    else
    {
        FILE * fp;
        strcpy(glideIni, envpath);
        if (glideIni[strlen(glideIni)-1] != '/') strcat(glideIni, "/");
        strcat(glideIni, ".marley/mupen64plus/Glide64mk2.ini");
        // if glideIni not exists, create it
        if (stat(glideIni, &fileinfo) != 0)
        {
            printf("creating %s\n",glideIni);
            fp = fopen (glideIni,"w");
            outputGlideIni(fp);
            fclose (fp);
        }
    }

    // Success
    return 0;
}

static int search_dir_file(char *destpath, const char *path, const char *filename)
{
    struct stat fileinfo;

    // sanity check to start
    if (destpath == NULL || path == NULL || filename == NULL)
        return 1;

    // build the full filepath
    strcpy(destpath, path);
    // if the path is empty, don't add / between it and the file name
    if (destpath[0] != '\0' && destpath[strlen(destpath)-1] != '/')
        strcat(destpath, "/");
    strcat(destpath, filename);

    // test for a valid file
    if (stat(destpath, &fileinfo) != 0)
        return 2;
    if (!S_ISREG(fileinfo.st_mode))
        return 3;

    // success - file exists and is a regular file
    return 0;
}

// global functions

int osal_mkdirp(const char *dirpath, int mode)
{
    char *mypath, *currpath;
    struct stat fileinfo;

    // Terminate quickly if the path already exists
    if (stat(dirpath, &fileinfo) == 0 && S_ISDIR(fileinfo.st_mode))
        return 0;

    // Create partial paths
    mypath = currpath = strdup(dirpath);
    if (mypath == NULL)
        return 1;

    while ((currpath = strpbrk(currpath + 1, OSAL_DIR_SEPARATORS)) != NULL)
    {
        *currpath = '\0';
        if (stat(mypath, &fileinfo) != 0)
        {
            if (mkdir(mypath, mode) != 0)
                break;
        }
        else
        {
            if (!S_ISDIR(fileinfo.st_mode))
                break;
        }
        *currpath = OSAL_DIR_SEPARATORS[0];
    }
    free(mypath);
    if (currpath != NULL)
        return 1;

    // Create full path
    if (stat(dirpath, &fileinfo) != 0 && mkdir(dirpath, mode) != 0)
        return 1;

    return 0;
}

const char * osal_get_shared_filepath(const char *filename, const char *firstsearch, const char *secondsearch)
{
    static char retpath[PATH_MAX];
    int i;

    // if caller gave us any directories to search, then look there first
    if (firstsearch != NULL && search_dir_file(retpath, firstsearch, filename) == 0)
        return retpath;
    if (secondsearch != NULL && search_dir_file(retpath, secondsearch, filename) == 0)
        return retpath;

    // otherwise check our standard paths
    for (i = 0; i < datasearchdirs; i++)
    {
        if (search_dir_file(retpath, datasearchpath[i], filename) == 0)
            return retpath;
    }

    // we couldn't find the file
    return NULL;
}

const char * osal_get_user_configpath(void)
{
    static char retpath[PATH_MAX];
    int rval;
    
    #warning "JC: modified"

    // then try the HOME environment variable
    rval = get_xdg_dir(retpath, "HOME", ".marley/mupen64plus/");
    if (rval == 0)
    {
        return retpath;
    }

    // otherwise we are in trouble
    if (rval < 3)
        DebugMessage(M64MSG_ERROR, "Failed to get configuration directory; $HOME is undefined or invalid.");
    return NULL;
}

const char * osal_get_user_datapath(void)
{
    static char retpath[PATH_MAX];
    struct stat fileinfo;
    const char *envpath = getenv("HOME");
    
    #warning "JC: modified"
    
    // error if this environment variable doesn't return a good string
    if (envpath == NULL || strlen(envpath) < 1)
        return NULL;

    // error if path returned by the environemnt variable isn't a valid path to a directory
    if (stat(envpath, &fileinfo) != 0 || !S_ISDIR(fileinfo.st_mode))
        return NULL;

    // append the given sub-directory to the path given by the environment variable
    strcpy(retpath, envpath);
    if (retpath[strlen(retpath)-1] != '/')
        strcat(retpath, "/");
    strcat(retpath, ".marley/mupen64plus/");

    return retpath;
}

const char * osal_get_user_cachepath(void)
{
    static char retpath[PATH_MAX];
    int rval;

    // then try the HOME environment variable
    rval = get_xdg_dir(retpath, "HOME", ".marley/mupen64plus/");
    if (rval == 0)
    {
        return retpath;
    }

    // otherwise we are in trouble
    if (rval < 3)
        DebugMessage(M64MSG_ERROR, "Failed to get cache directory; $HOME is undefined or invalid.");
    return NULL;
}


void outputGlideIni(FILE * fp)
{
    fprintf (fp, ";_____________________________________________________________________\n");
    fprintf (fp, "; SETTINGS:\n");
    fprintf (fp, "; This section contains the plugin settings, such as\n");
    fprintf (fp, "; resolution.\n");
    fprintf (fp, ";\n");
    fprintf (fp, "; resolution - specifies which resolution to use\n");
    fprintf (fp, ";  Resolutions are as follows:\n");
    fprintf (fp, "; 0  - 320, 200\n");
    fprintf (fp, "; 1  - 320, 240\n");
    fprintf (fp, "; 2  - 400, 256\n");
    fprintf (fp, "; 3  - 512, 384\n");
    fprintf (fp, "; 4  - 640, 200\n");
    fprintf (fp, "; 5  - 640, 350\n");
    fprintf (fp, "; 6  - 640, 400\n");
    fprintf (fp, "; 7  - 640, 480\n");
    fprintf (fp, "; 8  - 800, 600\n");
    fprintf (fp, "; 9  - 960, 720\n");
    fprintf (fp, "; 10 - 856, 480\n");
    fprintf (fp, "; 11 - 512, 256\n");
    fprintf (fp, "; 12 - 1024, 768\n");
    fprintf (fp, "; 13 - 1280, 1024\n");
    fprintf (fp, "; 14 - 1600, 1200\n");
    fprintf (fp, "; 15 - 400, 300\n");
    fprintf (fp, "; 16 - 1152, 864\n");
    fprintf (fp, "; 17 - 1280, 960\n");
    fprintf (fp, "; 18 - 1600, 1024\n");
    fprintf (fp, "; 19 - 1792, 1344\n");
    fprintf (fp, "; 20 - 1856, 1392\n");
    fprintf (fp, "; 21 - 1920, 1440\n");
    fprintf (fp, "; 22 - 2048, 1536\n");
    fprintf (fp, "; 23 - 2048, 2048\n");
    fprintf (fp, "; Note: some video cards or monitors do not support all\n");
    fprintf (fp, "; resolutions!\n");
    fprintf (fp, ";\n");
    fprintf (fp, "; Note#2:For compatibility issues always distribute this\n");
    fprintf (fp, "; file with the resolution: 640, 480 (7)  \n");
    fprintf (fp, ";\n");
    fprintf (fp, "\n");
    fprintf (fp, "[SETTINGS]\n");
    fprintf (fp, "hotkeys = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "; UCODE:\n");
    fprintf (fp, "; These are ucode crcs used in the uCode detector.\n");
    fprintf (fp, "; If a crc is not found here, the plugin will ask you\n");
    fprintf (fp, "; to add it. All these values are in hexadecimal.\n");
    fprintf (fp, ";\n");
    fprintf (fp, "; uCodes:\n");
    fprintf (fp, "; -1 - Unknown, display error\n");
    fprintf (fp, "; 0 - RSP SW 2.0X (Super Mario 64)\n");
    fprintf (fp, "; 1 - F3DEX 1.XX (Star Fox 64)\n");
    fprintf (fp, "; 2 - F3DEX 2.XX (The Legend of Zelda: Ocarina of Time)\n");
    fprintf (fp, "; 3 - F3DEX ? (WaveRace)\n");
    fprintf (fp, "; 4 - RSP SW 2.0D EXT (Star Wars: Shadows of the Empire)\n");
    fprintf (fp, "; 5 - RSP SW 2.0 (Diddy Kong Racing); \n");
    fprintf (fp, "; 6 - S2DEX 1.XX  (Yoshi's Story - SimCity 2000)\n");
    fprintf (fp, "; 7 - RSP SW PD (Perfect Dark)\n");
    fprintf (fp, "; 8 - F3DEXBG 2.08 (Conker's Bad Fur Day)\n");
    fprintf (fp, "\n");
    fprintf (fp, "[UCODE]\n");
    fprintf (fp, "006bd77f=0\n");
    fprintf (fp, "03044b84=2\n");
    fprintf (fp, "030f4b84=2\n");
    fprintf (fp, "05165579=1\n");
    fprintf (fp, "05777c62=1\n");
    fprintf (fp, "057e7c62=1\n");
    fprintf (fp, "07200895=0\n");
    fprintf (fp, "0bf36d36=9\n");
    fprintf (fp, "0d7bbffb=-1 \n");
    fprintf (fp, "0d7cbffb=5\n");
    fprintf (fp, "0ff79527=2\n");
    fprintf (fp, "0ff795bf=-1 \n");
    fprintf (fp, "1118b3e0=1\n");
    fprintf (fp, "1517a281=1\n");
    fprintf (fp, "168e9cd5=2\n");
    fprintf (fp, "1a1e18a0=2\n");
    fprintf (fp, "1a1e1920=2\n");
    fprintf (fp, "1a62dbaf=2\n");
    fprintf (fp, "1a62dc2f=2\n");
    fprintf (fp, "1de712ff=1\n");
    fprintf (fp, "1ea9e30f=6\n");
    fprintf (fp, "1f120bbb=21\n");
    fprintf (fp, "21f91834=2\n");
    fprintf (fp, "21f91874=2\n");
    fprintf (fp, "22099872=10\n");
    fprintf (fp, "24cd885b=1\n");
    fprintf (fp, "26a7879a=1\n");
    fprintf (fp, "299d5072=6\n");
    fprintf (fp, "2b291027=2\n");
    fprintf (fp, "2b5a89c2=6\n");
    fprintf (fp, "2c7975d6=1\n");
    fprintf (fp, "2d3fe3f1=1\n");
    fprintf (fp, "2f71d1d5=2\n");
    fprintf (fp, "2f7dd1d5=2\n");
    fprintf (fp, "327b933d=1\n");
    fprintf (fp, "339872a6=1\n");
    fprintf (fp, "377359b6=2\n");
    fprintf (fp, "3a1c2b34=0\n");
    fprintf (fp, "3a1cbac3=0\n");
    fprintf (fp, "3f7247fb=0\n");
    fprintf (fp, "3ff1a4ca=1\n");
    fprintf (fp, "4165e1fd=0\n");
    fprintf (fp, "4340ac9b=1\n");
    fprintf (fp, "440cfad6=1\n");
    fprintf (fp, "47d46e86=7\n");
    fprintf (fp, "485abff2=2\n");
    fprintf (fp, "4fe6df78=1\n");
    fprintf (fp, "5182f610=0\n");
    fprintf (fp, "5257cd2a=1\n");
    fprintf (fp, "5414030c=1\n");
    fprintf (fp, "5414030d=1\n");
    fprintf (fp, "559ff7d4=1\n");
    fprintf (fp, "5b5d36e3=4\n");
    fprintf (fp, "5b5d3763=3\n");
    fprintf (fp, "5d1d6f53=0\n");
    fprintf (fp, "5d3099f1=2\n");
    fprintf (fp, "5df1408c=1\n");
    fprintf (fp, "5ef4e34a=1\n");
    fprintf (fp, "6075e9eb=1\n");
    fprintf (fp, "60c1dcc4=1\n");
    fprintf (fp, "6124a508=2\n");
    fprintf (fp, "630a61fb=2\n");
    fprintf (fp, "63be08b1=5\n");
    fprintf (fp, "63be08b3=5\n");
    fprintf (fp, "64ed27e5=1\n");
    fprintf (fp, "65201989=2\n");
    fprintf (fp, "65201a09=2\n");
    fprintf (fp, "66c0b10a=1\n");
    fprintf (fp, "679e1205=2\n");
    fprintf (fp, "6bb745c9=6\n");
    fprintf (fp, "6d8f8f8a=2\n");
    fprintf (fp, "6e4d50af=0\n");
    fprintf (fp, "6eaa1da8=1\n");
    fprintf (fp, "72a4f34e=1\n");
    fprintf (fp, "73999a23=1\n");
    fprintf (fp, "74af0a74=6\n");
    fprintf (fp, "753be4a5=2\n");
    fprintf (fp, "794c3e28=6\n");
    fprintf (fp, "7df75834=1\n");
    fprintf (fp, "7f2d0a2e=1\n");
    fprintf (fp, "82f48073=1\n");
    fprintf (fp, "832fcb99=1\n");
    fprintf (fp, "841ce10f=1\n");
    fprintf (fp, "844b55b5=-1\n");
    fprintf (fp, "863e1ca7=1\n");
    fprintf (fp, "86b1593e=-1\n");
    fprintf (fp, "8805ffea=1\n");
    fprintf (fp, "8d5735b2=1\n");
    fprintf (fp, "8d5735b3=1\n");
    fprintf (fp, "8ec3e124=-1\n");
    fprintf (fp, "93d11f7b=2\n");
    fprintf (fp, "93d11ffb=2\n");
    fprintf (fp, "93d1ff7b=2\n");
    fprintf (fp, "9551177b=2\n");
    fprintf (fp, "955117fb=2\n");
    fprintf (fp, "95cd0062=2\n");
    fprintf (fp, "97d1b58a=1\n");
    fprintf (fp, "a2d0f88e=2\n");
    fprintf (fp, "a346a5cc=1\n");
    fprintf (fp, "aa86cb1d=2\n");
    fprintf (fp, "aae4a5b9=2\n");
    fprintf (fp, "ad0a6292=2\n");
    fprintf (fp, "ad0a6312=2\n");
    fprintf (fp, "ae08d5b9=0\n");
    fprintf (fp, "b1821ed3=1\n");
    fprintf (fp, "b4577b9c=1\n");
    fprintf (fp, "b54e7f93=0\n");
    fprintf (fp, "b62f900f=2\n");
    fprintf (fp, "ba65ea1e=2\n");
    fprintf (fp, "ba86cb1d=8\n");
    fprintf (fp, "bc03e969=0\n");
    fprintf (fp, "bc45382e=2\n");
    fprintf (fp, "be78677c=1\n");
    fprintf (fp, "bed8b069=1\n");
    fprintf (fp, "c3704e41=1\n");
    fprintf (fp, "c46dbc3d=1\n");
    fprintf (fp, "c99a4c6c=1\n");
    fprintf (fp, "c901ce73=2\n");
    fprintf (fp, "c901cef3=2\n");
    fprintf (fp, "cb8c9b6c=2\n");
    fprintf (fp, "cee7920f=1\n");
    fprintf (fp, "cfa35a45=2\n");
    fprintf (fp, "d1663234=1\n");
    fprintf (fp, "d20dedbf=6\n");
    fprintf (fp, "d2a9f59c=1\n");
    fprintf (fp, "d41db5f7=1\n");
    fprintf (fp, "d5604971=0\n");
    fprintf (fp, "d57049a5=1\n");
    fprintf (fp, "d5c4dc96=-1\n");
    fprintf (fp, "d5d68b1f=0\n");
    fprintf (fp, "d67c2f8b=0\n");
    fprintf (fp, "d802ec04=1\n");
    fprintf (fp, "da13ab96=2\n");
    fprintf (fp, "de7d67d4=2\n");
    fprintf (fp, "e1290fa2=2\n");
    fprintf (fp, "e41ec47e=0\n");
    fprintf (fp, "e65cb4ad=2\n");
    fprintf (fp, "e89c2b92=1\n");
    fprintf (fp, "e9231df2=1\n");
    fprintf (fp, "ec040469=1\n");
    fprintf (fp, "ee47381b=1\n");
    fprintf (fp, "ef54ee35=1\n");
    fprintf (fp, "f9893f70=21\n");
    fprintf (fp, "fb816260=1\n");
    fprintf (fp, "ff372492=21\n");
    fprintf (fp, "\n");
    fprintf (fp, "\n");
    fprintf (fp, "\n");
    fprintf (fp, "; Game specific settings\n");
    fprintf (fp, ";\n");
    fprintf (fp, "; In the [DEFAULT] section there are the default options for a game, which can\n");
    fprintf (fp, "; be overriden in the section with the game's internal name.\n");
    fprintf (fp, "\n");
    fprintf (fp, "[DEFAULT]\n");
    fprintf (fp, "filtering = 0\n");
    fprintf (fp, "fog = 1\n");
    fprintf (fp, "buff_clear = 1\n");
    fprintf (fp, "swapmode = 1\n");
    fprintf (fp, "lodmode = 0\n");
    fprintf (fp, "fb_smart = 0\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_get_info = 0\n");
    fprintf (fp, "fb_render = 1\n");
    fprintf (fp, "fb_crc_mode = 1\n");
    fprintf (fp, "read_back_to_screen = 0\n");
    fprintf (fp, "detect_cpu_write = 0\n");
    fprintf (fp, "alt_tex_size = 0\n");
    fprintf (fp, "use_sts1_only = 0\n");
    fprintf (fp, "fast_crc = 1\n");
    fprintf (fp, "force_microcheck = 0\n");
    fprintf (fp, "force_quad3d = 0\n");
    fprintf (fp, "optimize_texrect = 1\n");
    fprintf (fp, "hires_buf_clear = 1\n");
    fprintf (fp, "fb_read_alpha = 0\n");
    fprintf (fp, "force_calc_sphere = 0\n");
    fprintf (fp, "texture_correction = 1\n");
    fprintf (fp, "increase_texrect_edge = 0\n");
    fprintf (fp, "decrease_fillrect_edge = 0\n");
    fprintf (fp, "stipple_mode = 2\n");
    fprintf (fp, "stipple_pattern = 1041204192\n");
    fprintf (fp, "clip_zmax = 1\n");
    fprintf (fp, "clip_zmin = 0\n");
    fprintf (fp, "adjust_aspect = 0\n");
    fprintf (fp, "correct_viewport = 0\n");
    fprintf (fp, "aspect = 0\n");
    fprintf (fp, "zmode_compare_less = 0\n");
    fprintf (fp, "old_style_adither = 0\n");
    fprintf (fp, "n64_z_scale = 0\n");
    fprintf (fp, "pal230 = 0\n");
    fprintf (fp, "ignore_aux_copy = 0\n");
    fprintf (fp, "useless_is_useless = 0\n");
    fprintf (fp, "fb_read_always = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[1080 SNOWBOARDING]\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[A Bug's Life]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[AERO FIGHTERS ASSAUL]\n");
    fprintf (fp, "clip_zmin = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[AIDYN_CHRONICLES]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[All-Star Baseball 20] \n");
    fprintf (fp, "\n");
    fprintf (fp, "[All-Star Baseball 99]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[All Star Baseball 99]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[All-Star Baseball '0]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[ARMYMENAIRCOMBAT]\n");
    fprintf (fp, "increase_texrect_edge = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BURABURA POYON]\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Bakushou Jinsei 64 - Mezease! Resort Ou.\n");
    fprintf (fp, "[ÊÞ¸¼®³¼ÞÝ¾²64]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BAKU-BOMBERMAN]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BAKUBOMB2]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BANGAIOH]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Banjo-Kazooie]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_always = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BANJO KAZOOIE 2]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_always = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BANJO TOOIE]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_always = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BASS HUNTER 64]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BATTLEZONE]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BEETLE ADVENTURE JP]\n");
    fprintf (fp, "n64_z_scale = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Beetle Adventure Rac]\n");
    fprintf (fp, "n64_z_scale = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Big Mountain 2000]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BIOFREAKS]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BioHazard II]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "adjust_aspect = 0\n");
    fprintf (fp, "n64_z_scale = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Blast Corps]\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Blastdozer]\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[blitz2k]\n");
    fprintf (fp, "lodmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Body Harvest]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BOMBERMAN64E]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BOMBERMAN64U]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BOMBERMAN64U2]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Bottom of the 9th]\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[BRUNSWICKBOWLING]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Bust A Move 3 DX]\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Bust A Move '99]\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Bust A Move 2]\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[CARMAGEDDON64]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[CASTLEVANIA]\n");
    fprintf (fp, "old_style_adither = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[CASTLEVANIA2]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[CENTRE COURT TENNIS]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Chameleon Twist2]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[CHOPPER_ATTACK]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[CITY TOUR GP]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Command&Conquer]\n");
    fprintf (fp, "aspect = 2\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[CONKER BFD]\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "lodmode = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Cruis'n USA]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[CruisnExotica]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[custom robo]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[CUSTOMROBOV2]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[CyberTiger]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[DAFFY DUCK STARRING]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[DARK RIFT]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[DeadlyArts]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "clip_zmin = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[DERBYSTALLION 64]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[D K DISPLAY]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Donald Duck Goin' Qu]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Donald Duck Quack At]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[DONKEY KONG 64]\n");
    fprintf (fp, "lodmode = 1\n");
    fprintf (fp, "fb_read_always = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Doom64]\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Doraemon - Mittsu no Seireiseki (J)\n");
    fprintf (fp, "[ÄÞ×´ÓÝ Ð¯ÂÉ¾²Ú²¾·]\n");
    fprintf (fp, "read_back_to_screen = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Doraemon 3 - Nobita no Machi SOS! (J)\n");
    fprintf (fp, "[ÄÞ×´ÓÝ3 ÉËÞÀÉÏÁSOS!]\n");
    fprintf (fp, "clip_zmin = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[DR.MARIO 64]\n");
    fprintf (fp, "read_back_to_screen = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[DRACULA MOKUSHIROKU]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[DRACULA MOKUSHIROKU2]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Dual heroes JAPAN]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Dual heroes PAL]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Dual heroes USA]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[DUKE NUKEM]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[EARTHWORM JIM 3D]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Eikou no Saint Andrew\n");
    fprintf (fp, "[´²º³É¾ÝÄ±ÝÄÞØ­°½]\n");
    fprintf (fp, "correct_viewport = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Eltail]\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[EVANGELION]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[EXCITEBIKE64]\n");
    fprintf (fp, "fb_smart = 0\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[extreme_g]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[extremeg]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[´¸½ÄØ°ÑG2]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Extreme G 2]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[F1 POLE POSITION 64]\n");
    fprintf (fp, "clip_zmin = 1\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[F1RacingChampionship]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[F1 WORLD GRAND PRIX]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[F1 WORLD GRAND PRIX2]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[F-ZERO X]\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Fushigi no Dungeon - Furai no Shiren 2 (J) \n");
    fprintf (fp, "[F3 Ì³×²É¼ÚÝ2]\n");
    fprintf (fp, "decrease_fillrect_edge = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Fighting Force]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[G.A.S.P!!Fighters'NE]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "clip_zmin = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[GANBAKE GOEMON]\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Ganbare Goemon - Neo Momoyama Bakufu no Odori\n");
    fprintf (fp, "[¶ÞÝÊÞÚ\\ ºÞ´ÓÝ]\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[GAUNTLET LEGENDS]\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Getter Love!!]\n");
    fprintf (fp, "zmode_compare_less = 1\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Gex 3 Deep Cover Gec]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[GEX: ENTER THE GECKO]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Glover]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[GOEMON2 DERODERO]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[GOEMONS GREAT ADV]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[GOLDENEYE]\n");
    fprintf (fp, "lodmode = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[GOLDEN NUGGET 64]\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[GT64]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "; Hamster Monogatori\n");
    fprintf (fp, "[ÊÑ½À°ÓÉ¶ÞÀØ64]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[HARVESTMOON64]\n");
    fprintf (fp, "zmode_compare_less = 1\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "; Harvest Moon 64 JAP\n");
    fprintf (fp, "[ÎÞ¸¼Þ®³ÓÉ¶ÞÀØ2]\n");
    fprintf (fp, "zmode_compare_less = 1\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "; Heiwa Pachinko World\n");
    fprintf (fp, "[HEIWA ÊßÁÝº Ü°ÙÄÞ64]\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[HEXEN]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[HSV ADVENTURE RACING]\n");
    fprintf (fp, "n64_z_scale = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Holy Magic Century]\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[HUMAN GRAND PRIX]\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[·×¯Ä¶²¹Â 64ÀÝÃ²ÀÞÝ]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Iggy's Reckin' Balls]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[I S S 64]\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "old_style_adither = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[I.S.S.2000]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[ITF 2000]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[IT&F SUMMERGAMES]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[J_league 1997]\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, ";J.League Eleven Beat 1997\n");
    fprintf (fp, "[JØ°¸Þ\\ ²ÚÌÞÝËÞ°Ä1997]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[J LEAGUE LIVE 64]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[J WORLD SOCCER3]\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[JEREMY MCGRATH SUPER]\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[JET FORCE GEMINI]\n");
    fprintf (fp, "read_back_to_screen = 1\n");
    fprintf (fp, "decrease_fillrect_edge = 1\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[J F G DISPLAY]\n");
    fprintf (fp, "read_back_to_screen = 1\n");
    fprintf (fp, "decrease_fillrect_edge = 1\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[KEN GRIFFEY SLUGFEST]\n");
    fprintf (fp, "read_back_to_screen = 2\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Kirby64]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Killer Instinct Gold]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[KNIFE EDGE]\n");
    fprintf (fp, "fast_crc = 0\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Knockout Kings 2000]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[LAMBORGHINI]\n");
    fprintf (fp, "use_sts1_only = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[LCARS - WT_Riker]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[LEGORacers]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[LET'S SMASH]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Lode Runner 3D] \n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[LT DUCK DODGERS]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MACE]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MAGICAL TETRIS]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Mahjong Master (J)\n");
    fprintf (fp, "[Ï°¼Þ¬Ý Ï½À°]\n");
    fprintf (fp, "n64_z_scale = 1\n");
    fprintf (fp, "zmode_compare_less = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MAJORA'S MASK]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_crc_mode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MARIOKART64]\n");
    fprintf (fp, "fast_crc = 0\n");
    fprintf (fp, "stipple_mode = 1\n");
    fprintf (fp, "stipple_pattern = 4286595040\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MarioGolf64]\n");
    fprintf (fp, "ignore_aux_copy = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MarioParty]\n");
    fprintf (fp, "clip_zmin = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MarioParty2]\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MarioParty3]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MARIO STORY]\n");
    fprintf (fp, "useless_is_useless = 1\n");
    fprintf (fp, "hires_buf_clear = 0\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MASTERS'98]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Mega Man 64]\n");
    fprintf (fp, "increase_texrect_edge = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MGAH VOL1]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "zmode_compare_less = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Mia Hamm Soccer 64]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MICKEY USA]\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MICKEY USA PAL]\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MICROMACHINES64TURBO]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Mini Racers]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MISCHIEF MAKERS]\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MLB FEATURING K G JR]\n");
    fprintf (fp, "read_back_to_screen = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MK_MYTHOLOGIES]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MO WORLD LEAGUE SOCC]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Monaco GP Racing 2]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Monaco Grand Prix]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Morita Shogi 64\n");
    fprintf (fp, "[ÓØÀ¼®³·Þ64]\n");
    fprintf (fp, "correct_viewport = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MortalKombatTrilogy]\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MS. PAC-MAN MM]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MYSTICAL NINJA]\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MYSTICAL NINJA2 SG]\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NASCAR 99]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NASCAR 2000]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NBA Courtside 2]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NBA JAM 2000]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NBA JAM 99]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NBA LIVE 2000]\n");
    fprintf (fp, "adjust_aspect = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NBA Live 99]\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "adjust_aspect = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NEWTETRIS]\n");
    fprintf (fp, "pal230 = 1\n");
    fprintf (fp, "increase_texrect_edge = 1\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NFL BLITZ]\n");
    fprintf (fp, "lodmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NFL BLITZ 2001]\n");
    fprintf (fp, "lodmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NFL BLITZ SPECIAL ED]\n");
    fprintf (fp, "lodmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NFL QBC '99]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NFL QBC 2000]\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NFL Quarterback Club]\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NINTAMAGAMEGALLERY64]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NITRO64] \n");
    fprintf (fp, "fb_smart = 1 \n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[NUCLEARSTRIKE64]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "; Nushi Zuri 64\n");
    fprintf (fp, "[Ç¼ÂÞØ64]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[OgreBattle64]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[PACHINKO365NICHI]\n");
    fprintf (fp, "correct_viewport = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[PAPER MARIO]\n");
    fprintf (fp, "useless_is_useless = 1\n");
    fprintf (fp, "hires_buf_clear = 0\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Parlor PRO 64]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Perfect Dark]\n");
    fprintf (fp, "useless_is_useless = 1\n");
    fprintf (fp, "decrease_fillrect_edge = 1\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[PERFECT STRIKER]\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Pilot Wings64]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[PUZZLE LEAGUE N64]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 0\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[PUZZLE LEAGUE]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 0\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[POKEMON SNAP]\n");
    fprintf (fp, "fast_crc = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 0\n");
    fprintf (fp, "fb_read_always = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[POKEMON STADIUM]\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "fast_crc = 0\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 0\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "fb_crc_mode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[POKEMON STADIUM 2]\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fast_crc = 0\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "fb_crc_mode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[POKEMON STADIUM G&S]\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "fast_crc = 0\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 0\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "fb_crc_mode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[POLARISSNOCROSS]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[PowerLeague64]\n");
    fprintf (fp, "force_quad3d = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Quake]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[QUAKE II]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[quarterback_club_98]\n");
    fprintf (fp, "optimize_texrect = 0\n");
    fprintf (fp, "hires_buf_clear = 0\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_read_alpha = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Quest 64]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Racing Simulation 2] \n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[RAINBOW SIX]\n");
    fprintf (fp, "increase_texrect_edge = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Rally'99]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[RALLY CHALLENGE]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Rayman 2]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[READY 2 RUMBLE]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Ready to Rumble]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Resident Evil II]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "adjust_aspect = 0\n");
    fprintf (fp, "n64_z_scale = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Re-Volt]\n");
    fprintf (fp, "texture_correction = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[RIDGE RACER 64]\n");
    fprintf (fp, "force_calc_sphere = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[ROAD RASH 64]\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Robopon64]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[ROCKETROBOTONWHEELS]\n");
    fprintf (fp, "clip_zmin = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[RockMan Dash]\n");
    fprintf (fp, "increase_texrect_edge = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[RONALDINHO SOCCER]\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "old_style_adither = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[RTL WLS2000]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[RUGRATS IN PARIS]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[RUSH 2049]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[SCARS]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[SD HIRYU STADIUM]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Shadow of the Empire]\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Shadowman]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Silicon Valley]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Snobow Kids 2]\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[SNOWBOARD KIDS2]\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[South Park: Chef's L]\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[South Park Chef's Lu]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[SPACE DYNAMITES] \n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[SPIDERMAN]\n");
    fprintf (fp, "fast_crc = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[STARCRAFT 64]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "aspect = 2\n");
    fprintf (fp, "filtering = 2\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[STAR SOLDIER]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[STAR TWINS]\n");
    fprintf (fp, "read_back_to_screen = 1\n");
    fprintf (fp, "decrease_fillrect_edge = 1\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[STAR WARS EP1 RACER]\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[SUPERROBOTSPIRITS]\n");
    fprintf (fp, "aspect = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Super Robot Taisen 64 (J)\n");
    fprintf (fp, "[½°Êß°ÛÎÞ¯ÄÀ²¾Ý64]\n");
    fprintf (fp, "fast_crc = 0\n");
    fprintf (fp, "use_sts1_only = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Supercross]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[SUPER MARIO 64]\n");
    fprintf (fp, "lodmode = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[SUPERMARIO64]\n");
    fprintf (fp, "lodmode = 1\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[SUPERMAN]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Susume! Taisen Puzzle Dama Toukon! Marumata Chou (J) \n");
    fprintf (fp, "[½½Ò!À²¾ÝÊß½ÞÙÀÞÏ]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, ";Tamagotchi World 64 (J) \n");
    fprintf (fp, "[ÐÝÅÃÞÀÏºÞ¯ÁÜ°ÙÄÞ]\n");
    fprintf (fp, "use_sts1_only = 1\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Taz Express]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TELEFOOT SOCCER 2000]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TETRISPHERE]\n");
    fprintf (fp, "alt_tex_size = 1\n");
    fprintf (fp, "use_sts1_only = 1\n");
    fprintf (fp, "increase_texrect_edge = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_crc_mode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TG RALLY 2]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[THE LEGEND OF ZELDA]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[THE MASK OF MUJURA]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_crc_mode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[THPS2]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[THPS3]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Tigger's Honey Hunt]\n");
    fprintf (fp, "zmode_compare_less = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TOM AND JERRY]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Tonic Trouble]\n");
    fprintf (fp, "detect_cpu_write = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TONY HAWK PRO SKATER]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TONY HAWK SKATEBOARD]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Top Gear Hyper Bike]\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Top Gear Overdrive]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TOP GEAR RALLY]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TOP GEAR RALLY 2]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TRIPLE PLAY 2000]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TROUBLE MAKERS]\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TSUMI TO BATSU]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TSUWAMONO64]\n");
    fprintf (fp, "force_microcheck = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TWINE]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[TWISTED EDGE]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Ultraman Battle JAPA]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Virtual Pool 64]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[V-RALLY]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Waialae Country Club]\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[WAVE RACE 64]\n");
    fprintf (fp, "pal230 = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[WILD CHOPPERS]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[Wipeout 64]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "swapmode = 2\n");
    fprintf (fp, "\n");
    fprintf (fp, "[WONDER PROJECT J2]\n");
    fprintf (fp, "buff_clear = 0\n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[World Cup 98] \n");
    fprintf (fp, "swapmode = 0\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "[WRESTLEMANIA 2000]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[YAKOUTYUU2]\n");
    fprintf (fp, "\n");
    fprintf (fp, "[YOSHI STORY]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fog = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[ZELDA MAJORA'S MASK]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "fb_crc_mode = 0\n");
    fprintf (fp, "\n");
    fprintf (fp, "[ZELDA MASTER QUEST]\n");
    fprintf (fp, "filtering = 1\n");
    fprintf (fp, "fb_smart = 1\n");
    fprintf (fp, "fb_hires = 1\n");
    fprintf (fp, "\n");
    fprintf (fp, "\n");
    fprintf (fp, "[MUPEN64PLUS]\n");
    fprintf (fp, "\n");
    fprintf (fp, ";End Of Original File\n");

    
}
    
