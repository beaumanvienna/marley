/*  $Id: mdf2iso.c, 22/05/05 

    Copyright (C) 2004,2005 Salvatore Santagati <salvatore.santagati@gmail.com>   

    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   

    This program is distributed in the hope that it will be useful,       
    but WITHOUT ANY WARRANTY; without even the implied warranty of        
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
    GNU General Public License for more details.                          

    You should have received a copy of the GNU General Public License     
    along with this program; if not, write to the                         
    Free Software Foundation, Inc.,                                       
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.        
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define VERSION "0.3.1"


const char SYNC_HEADER[12] = { (char) 0x00,
  (char) 0xFF,
  (char) 0xFF,
  (char) 0xFF,
  (char) 0xFF,
  (char) 0xFF,
  (char) 0xFF,
  (char) 0xFF,
  (char) 0xFF,
  (char) 0xFF,
  (char) 0xFF,
  (char) 0x00
};

const char SYNC_HEADER_MDF_AUDIO[12] = { (char) 0x80,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80,
  (char) 0xC0,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80
};

const char SYNC_HEADER_MDF[12] = { (char) 0x80,
  (char) 0xC0,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80,
  (char) 0xC0,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80,
  (char) 0x80
};

const char ISO_9660[8] = { (char) 0x01,
  (char) 0x43,
  (char) 0x44,
  (char) 0x30,
  (char) 0x30,
  (char) 0x31,
  (char) 0x01,
  (char) 0x00
};

#define ISO9660 0
#define SYNC 1
#define SYNC_MDF 2
#define MDF_AUDIO 3
#define UNKNOWN -1

int toc_file (char *destfilename, int sub)
{
  int ret=0;
  char *destfiletoc;
  char *destfiledat;
  FILE *ftoc;
  
  destfiletoc=strdup(destfilename);
  destfiledat=strdup(destfilename);
  strcpy (destfiletoc + strlen (destfilename) - 4, ".toc");
  strcpy (destfiledat + strlen (destfilename) - 4, ".dat");

  if ((ftoc = fopen (destfiletoc, "w")) != nullptr)
    {
      fprintf (ftoc, "CD_ROM\n");
      fprintf (ftoc, "// Track 1\n");
      fprintf (ftoc, "TRACK MODE1_RAW");

    if (sub == 1) fprintf (ftoc, " RW_RAW\n");
    else fprintf (ftoc, "\n");

      fprintf (ftoc, "NO COPY\n");
      fprintf (ftoc, "DATAFILE \"%s\"\n", destfiledat);
      rename (destfilename, destfiledat);
      printf ("Create TOC File : %s\n", destfiletoc);
      fclose (ftoc);
    }
  else
    {
    printf ("Error opening %s for output: %s\n",destfiletoc,strerror(errno));
    ret=-1;
    };
  free(destfiletoc);
  free(destfiledat);
  return ret;
}

/*
int number_file (char *destfilename)
{
  int i = 1, test_mdf = 0;
  int n_mdf;
  char mdf[2], destfilemdf[2354];
  FILE *fsource;
  
  strcpy (destfilemdf, destfilename);
  strcpy (destfilemdf + strlen (destfilename) - 1, ".0");
  for (i = 0; test_mdf == 0; i++)
    {
      if ((fsource = fopen (destfilemdf, "rb")) != nullptr)
	{
	  printf ("\nCheck : ");
	  sprintf (mdf, "md%d", i);
	  strcpy (destfilemdf + strlen (destfilename) - 3, mdf);
	  printf ("%s, ", destfilemdf);
	  fclose (fsource);
	}
      else
	{
	  test_mdf = 1;
	}
  }
  printf ("\r                                   \n");
  n_mdf = i - 1;
  return (n_mdf);
}
*/

int cuesheets (char *destfilename,char *destfilename_no_path)
{
  int ret=0;
  char *destfilecue;
  char *destfilebin;
  FILE *fcue;
  
  destfilecue=strdup(destfilename);
  destfilebin=strdup(destfilename);
  strcpy (destfilecue + strlen (destfilename) - 4, ".cue");
  strcpy (destfilebin + strlen (destfilename) - 4, ".bin");
  if ((fcue = fopen (destfilecue, "w"))!=nullptr)
  {
  fprintf (fcue, "FILE \"%s\" BINARY\n", destfilename_no_path);
  fprintf (fcue, "TRACK 1 MODE1/2352\n");
  fprintf (fcue, "INDEX 1 00:00:00\n");
  rename (destfilename, destfilebin);
  printf ("Create Cuesheets : %s\n", destfilecue);
  fclose (fcue);
  }
  else
  {
    printf ("Error opening %s for output: %s\n",destfilecue,strerror(errno));
    ret=-1;
  }
  return ret;
}

int previous_percent=-1;
void main_percent (int percent_bar)
// Prints a progress bar, takes a percentage as argument.
{
  //int progress_bar, progress_space;
 
  if (percent_bar==previous_percent) return;  // Nothing changed, don't waste CPU cycles.
  
  printf("%3d%% [:%.*s>%.*s:]\r",percent_bar,20-(percent_bar/5),"                    ",
                                                  percent_bar/5,"====================");
  /*
  printf ("%d%% [:", percent_bar);
  for (progress_bar = 1; progress_bar <= (int) (percent_bar / 5); progress_bar++)
    printf ("=");
  printf (">");

  for (; progress_bar <= 20; ++progress_bar) printf (" ");
  printf (":]\r");
  */
}

void usage ()
// Prints the command line syntax
{
  printf (" Web     : http//mdf2iso.berlios.de\n");
  printf (" Email   : salvatore.santagati@gmail.com\n");
  printf (" Irc     : irc.freenode.net #ignus\n");
  printf (" Note    : iodellavitanonhocapitouncazzo\n\n");
  printf ("Usage :\n");
  printf ("mdf2iso [--cue|--toc|--help] <sourcefile> [destfile]\n\n");
  printf ("Options:\n");
  printf ("  --toc    Generate toc/dat files\n");
  printf ("  --cue    Generate cue/bin files\n");
  printf ("  --help   Display this notice\n");
  printf (" sourcefile\tFilename of the .mdf file to be converted\n");
  printf (" destfile\tFilename of the target ISO9660 file.\n"\
          "\t\tIf none given, one is derived from sourcefile.\n\n");
}

char mdftype(FILE *f)
/* returns 
   -1 for unknown
   0 for ISO9660
   1 for SYNC
   2 for SYNC MDF
   3 for MDF Audio
   (see #defines above)
*/
{
  char buf[12];
  
  fseek(f, 32768, SEEK_SET);
  fread(buf, sizeof (char), 8, f);
  if (!memcmp (ISO_9660, buf, 8)) return ISO9660; // File is ISO9660
  
  fseek(f, 0L, SEEK_SET);
  fread(buf, sizeof (char), 12, f);
  
  fseek (f, 2352, SEEK_SET);
  
  if (!memcmp (SYNC_HEADER, buf, 12))  // Has SYNC_HEADER
  {
    fread (buf, sizeof (char), 12, f);
    if (!memcmp (SYNC_HEADER_MDF, buf, 12)) return SYNC_MDF;   // File is SYNC MDF
    if (!memcmp (SYNC_HEADER, buf, 12)) return SYNC;       // File is SYNC
  }
  else  // Does not have SYNC_HEADER
  {
    fread(buf, sizeof (char), 12, f);
    if (!memcmp (SYNC_HEADER_MDF_AUDIO, buf, 12)) return MDF_AUDIO; // File is MDF Audio
  }

  // Reached a point where nothing else matters.  
  return UNKNOWN;  // Unknown format
}


// === Main program code ===

int mdf2iso_main (int argc, char **argv)
{
    int seek_ecc, sector_size, seek_head, sector_data;//, n_mdf;
    int cue = 0, cue_mode = 0, sub = 1, toc = 0, sub_toc = 0;
    int opts = 0;
    long i, source_length;
    char buf[2448];
    char *destfilename=nullptr;
    char *basefilename=nullptr;
    char *destfilename_no_path=nullptr;
    FILE *fdest, *fsource;

    // Print identification
    printf ("mdf2iso v%s by Salvatore Santagati\n", VERSION);
    printf ("Licensed under GPL v2 or later\n");


    // *** Process command line options ***

    // Search for options --cue
    for (i = 0; i < argc; i++)
	{
        if (!strcmp (argv[i], "--cue"))
	    {
            cue = 1;
            opts++;
	    }
	}

    // Catch impossible parameter combinations
    if (((cue == 1) && (toc == 1)) || ((opts == 1) && (argc <= 2)))
	{
        usage();
        return -1;
	}

    // Get the base filename
    basefilename=argv[1+opts];  
    destfilename_no_path=strdup(argv[2 + opts]);
    
    // Derive destination filename from the basename:
    //  If basename is "*.mdf" use "*.iso" als destname
    //  otherwise simply append ".iso" to the basename to create destname.
    destfilename=strdup(basefilename);
    i=strlen(destfilename);  
    if (i < 5 || strcmp(destfilename + i - 4, ".mdf"))
    {
        destfilename=(char*)realloc(destfilename,i+5); 
        strcat(destfilename, ".iso");
    }
    else
    {
        strcpy(destfilename + i - 3, "iso");
    }


    // *** Preprocess basefile ***
  

    // Try opening basefile
    if ((fsource = fopen(basefilename, "rb")) == nullptr)
    {
        free(destfilename);
        printf ("Could not open %s: %s\n", basefilename, strerror(errno));
        return -1;
    }

    // Determine filetype & set some stuff accordingly (or exit)
    switch (mdftype(fsource))
    {
        case ISO9660:
            printf("%s is already ISO9660.\n",basefilename);
            fclose(fsource);
            free(destfilename);
            return 0;
        case SYNC: 
            if (cue == 1)
			{
                cue_mode = 1;
			    sub = 0;
			    seek_ecc = 0;
                sector_size = 2352;
                sector_data = 2352;
                seek_head = 0;
            }
            if (toc == 0)
			{
                //NORMAL IMAGE
                seek_ecc = 288;
			    sector_size = 2352;
			    sector_data = 2048;
			    seek_head = 16;
            }
			else
			{
			   seek_ecc = 0;
			   sector_size = 2352;
			   sector_data = 2352;
			   seek_head = 0;
			}
            break;
        case SYNC_MDF:
            if (cue == 1)
            {
                cue_mode = 1;

                // BAD SECTOR TO NORMAL IMAGE
                seek_ecc = 96;
                sector_size = 2448;
                sector_data = 2352;
                seek_head = 0;
            }
            else if (toc == 0)
		    {
                // BAD SECTOR
                seek_ecc = 384;
                sector_size = 2448;
                sector_data = 2048;
                seek_head = 16;
		    }
            else
		    {
                //BAD SECTOR
                seek_ecc = 0;
                sector_size = 2448;
                sector_data = 2448;
                seek_head = 0;
                sub_toc = 1;
            }
            break;
        case MDF_AUDIO:
            //BAD SECTOR AUDIO
		    seek_head = 0;
		    sector_size = 2448;
		    seek_ecc = 96;
		    sector_data = 2352;
		    cue = 0;
            break;
        default:
            printf("Unknown format for %s.\n",basefilename);
            fclose(fsource);
            free(destfilename);
            return -1;
    }

    //  *** Create destination file ***
    
    // Try opening the destination file for output
    if ((fdest = fopen (destfilename, "wb")) == nullptr)
    {
        printf ("Unable to open %s for output: %s\n",destfilename,strerror(errno));
        free(destfilename);
        fclose(fsource);
        return -1;
    }

    
    fseek (fsource, 0L, SEEK_END);
	source_length = ftell (fsource) / sector_size;
	fseek (fsource, 0L, SEEK_SET);
	
	for (i = 0; i < source_length; i++)
	{
        fseek (fsource, seek_head, SEEK_CUR);
        if (fread(buf, sizeof (char), sector_data, fsource)!=sector_data)
		{
            printf ("Error reading from %s: %s\n",basefilename, strerror (errno));
            fclose(fsource);
            fclose(fdest);
            remove(destfilename);
            free(destfilename);
			return -1;
        }
        if (fwrite (buf, sizeof (char), sector_data, fdest)!=sector_data)
		{
            printf ("Error writing to %s: %s\n",destfilename, strerror (errno));
            fclose(fsource);
            fclose(fdest);
            remove(destfilename);
            free(destfilename);
			return -1;
        }
		fseek (fsource, seek_ecc, SEEK_CUR);
        main_percent(i*100/source_length);
    }
    printf ("100%% [:=====================:]\n");

	fclose (fsource);
	fclose (fdest);

    // *** create Toc or Cue file is requested ***
    if (cue == 1) if (cuesheets(destfilename,destfilename_no_path))
    {
        free(destfilename);
        return -1;
    }
    if (toc == 1) if (toc_file(destfilename, sub_toc))
    {
        free(destfilename);
        return -1;
    }
    if ((toc == 0) && (cue == 0))
    printf("Created iso9660: %s\n", destfilename);

    free(destfilename);
    return 0;

}
