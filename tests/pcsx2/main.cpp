#include <stdio.h>
#include <string>
#include <iostream>
#include <cstring>

using namespace std;

int pcsx2_main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    
    int pcsx2_argc;
    char *pcsx2_argv[5]; 
    char arg1[1024]; 
    char arg2[1024];
    char arg3[1024]; 
    char arg4[1024]; 
    char arg5[1024]; 
    int n;
    string str;
    
    
    str = "pcsx2";
    n = str.length(); 
    strcpy(arg1, str.c_str()); 
#define TEST1
#ifdef TEST1
    pcsx2_argc = 1;
    pcsx2_argv[0] = arg1;
#else
    
    str = "--nogui";
    n = str.length(); 
    strcpy(arg2, str.c_str()); 
    
    str = "--fullboot";
    n = str.length(); 
    strcpy(arg3, str.c_str()); 
    
    str = "--fullscreen";
    n = str.length(); 
    strcpy(arg4, str.c_str()); 
    
    str = argv[1];
    n = str.length(); 
    strcpy(arg5, str.c_str()); 

    pcsx2_argv[0] = arg1;
    pcsx2_argv[1] = arg2;
    pcsx2_argv[2] = arg3;
    pcsx2_argv[3] = arg4;
    
    if (argc > 1)
    {
        pcsx2_argv[4] = arg5;
        pcsx2_argc = 5;
    }
    else
    {
        pcsx2_argc = 4;
    }
#endif
    
    pcsx2_main(pcsx2_argc,pcsx2_argv);
    pcsx2_main(pcsx2_argc,pcsx2_argv);
    
    return 0;
}
