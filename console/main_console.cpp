//  main.m dcm2niix
// by Chris Rorden on 3/22/14, released under Gnu General Public License, Version 2.
//  Copyright (c) 2014 Chris Rorden. All rights reserved.

//g++ -O3 main_console.cpp nii_dicom.cpp nifti1_io_core.cpp nii_ortho.cpp nii_dicom_batch.cpp -s -o dcm2niix -lz

//if you do not have zlib,you can compile without it 
// g++ -O3 -DmyDisableZLib main_console.cpp nii_dicom.cpp nifti1_io_core.cpp nii_ortho.cpp nii_dicom_batch.cpp -s -o dcm2niix 
//or you can build your own copy:
// to compile you will first want to build the Z library, then compile the project
// cd zlib-1.2.8
// sudo ./configure;
// sudo make


//to generate combined 32-bit and 64-bit builds for OSX :
// g++ -O3 -x c++ main_console.c nii_dicom.c nifti1_io_core.c nii_ortho.c nii_dicom_batch.c -s -arch x86_64 -o dcm2niix64 -lz
// g++ -O3 -x c++ main_console.c nii_dicom.c nifti1_io_core.c nii_ortho.c nii_dicom_batch.c -s -arch i386 -o dcm2niix32 -lz
// lipo -create dcm2niix32 dcm2niix64 -o dcm2niix

//On windows with mingw you may get "fatal error: zlib.h: No such file
// to remedy, run "mingw-get install libz-dev" from mingw

//#define mydebugtest //automatically process directory specified in main, ignore input arguments


#include <stdlib.h>
#include <sys/stat.h>
//#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <float.h>
//#include <unistd.h>
#include <time.h>  // clock_t, clock, CLOCKS_PER_SEC
#include <stdio.h>
#include "nii_dicom_batch.h"
#include "nii_dicom.h"
#include "nifti1_io_core.h"
#include <math.h>

const char* removePath(const char* path) { // "/usr/path/filename.exe" -> "filename.exe"
    const char* pDelimeter = strrchr (path, '\\');
    if (pDelimeter)
        path = pDelimeter+1;
    pDelimeter = strrchr (path, '/');
    if (pDelimeter)
        path = pDelimeter+1;
    return path;
} //removePath()

void showHelp(const char * argv[], struct TDCMopts opts) {
    const char *cstr = removePath(argv[0]);
    printf("usage: %s [options] <in_folder>\n", cstr);
    printf(" Options :\n");
    printf("  -h : show help\n");
    printf("  -f : filename (%%c=comments %%f=folder name %%p=protocol %%i ID of patient %%n=name of patient %%s=series, %%t=time; default '%s')\n",opts.filename);
    printf("  -o : output directory (omit to save to input folder)\n");
    char gzCh = 'n';
    if (opts.isGz) gzCh = 'n';
    printf("  -z : gz compress images (y/n, default %c)\n", gzCh);
#if defined(_WIN64) || defined(_WIN32)
    printf(" Defaults stored in Windows registry\n");
    printf(" Examples :\n");
    printf("  %s c:\\DICOM\\dir\n", cstr);
    printf("  %s -o c:\\out\\dir c:\\DICOM\\dir\n", cstr);
    printf("  %s -f mystudy%%s c:\\DICOM\\dir\n", cstr);
    printf("  %s -o \"c:\\dir with spaces\\dir\" c:\\dicomdir\n", cstr);
#else
    printf(" Defaults file : %s\n", opts.optsname);
    printf(" Examples :\n");
    printf("  %s /Users/chris/dir\n", cstr);
    printf("  %s -o /users/cr/outdir/ -z y ~/dicomdir\n", cstr);
    printf("  %s -f mystudy%%s ~/dicomdir\n", cstr);
    printf("  %s -o \"~/dir with spaces/dir\" ~/dicomdir\n", cstr);
#endif
} //showHelp()

  
int main(int argc, const char * argv[])
{
    struct TDCMopts opts;
    readIniFile(&opts, argv);
#ifdef mydebugtest
    //strcpy(opts.indir, "/Users/rorden/desktop/sliceOrder/dicom2/Philips_PARREC_Rotation/NoRotation/DBIEX_4_1.PAR");
     strcpy(opts.indir, "/Users/rorden/desktop/sliceOrder/dicom2/test");
#else
    printf("Chris Rorden's dcm2niiX version %s\n",kDCMvers);
    if (argc < 2) {
        showHelp(argv, opts);
        return 0;
    }
    strcpy(opts.indir,argv[argc-1]);
    strcpy(opts.outdir,opts.indir);
    int i = 1;
    int lastCommandArg = -1;
    while (i < (argc)) { //-1 as final parameter is DICOM directory
        if ((strlen(argv[i]) > 1) && (argv[i][0] == '-')) { //command
            if (argv[i][1] == 'h')
                showHelp(argv, opts);
            else if ((argv[i][1] == 'z') && ((i+1) < argc)) {
                i++;
                if ((argv[i][0] == 'i') || (argv[i][0] == 'I') ) {
                    opts.isGz = true; //force use of internal compression instead of pigz
                	strcpy(opts.pigzname,"");
                } else if ((argv[i][0] == 'n') || (argv[i][0] == 'N')  || (argv[i][0] == '0'))
                    opts.isGz = false;
                else
                    opts.isGz = true;
            } else if ((argv[i][1] == 'f') && ((i+1) < argc)) {
                i++;
                strcpy(opts.filename,argv[i]);
            } else if ((argv[i][1] == 'o') && ((i+1) < argc)) {
                i++;
                strcpy(opts.outdir,argv[i]);
            }
            lastCommandArg = i;
            
        } //if parameter is a command
        i ++; //read next parameter
    } //while parameters to read
    //printf("%d %d",argc,lastCommandArg);
    if (argc == (lastCommandArg+1))  { //+1 as array indexed from 0
        //the user did not provide an input filename, report filename structure
        char niiFilename[1024];
        strcpy(opts.outdir,"");//no input supplied
        nii_createDummyFilename(niiFilename, opts);
        printf("%s\n",niiFilename);
        return EXIT_SUCCESS;
    }
#endif
    clock_t start = clock();
    nii_loadDir(&opts);
    printf ("Conversion required %f seconds.\n",((float)(clock()-start))/CLOCKS_PER_SEC);
    saveIniFile(opts);
    return EXIT_SUCCESS;
}
