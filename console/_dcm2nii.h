#ifndef __DCM2NII_INTERNAL_HEADER__
#define __DCM2NII_INTERNAL_HEADER__

#include <stdio.h>
#include <stdarg.h>
#include <float.h>

#if defined(_MSC_VER)
# include <windows.h>
#include <direct.h>
#else
# include <stdint.h>
# include <stdbool.h>
# include <unistd.h>
#endif

#if defined(_MSC_VER)
#define __DCM2NII_STRUCT_ATTR__ 
#else
#define __DCM2NII_STRUCT_ATTR__ __attribute__((packed))
#endif

inline int is_fileexists(const char * filename) {
  FILE * fp = NULL;
  if ((fp = fopen(filename, "r"))) {
    fclose(fp);
    return 1;
  }
  return 0;
}

int isDir(char * path);

#ifdef _MSC_VER

#define INFINITY (DBL_MAX+DBL_MAX)
#define NAN (INFINITY-INFINITY)
inline int isnan(double v){return v==NAN;}

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned __int32 uint32_t;
typedef signed __int32 int32_t;
typedef long long unsigned int uint64_t;

#define W_OK 1
inline int access(char * fname,int stat){
  if (stat==W_OK && isDir(fname)){return 0;}else{return 1;}
}
inline char * getcwd(char * path, int len){
  return _getcwd(path,len);
}
inline int round( double r ) {
  return (r > 0.0) ? (r + 0.5) : (r - 0.5); 
}
inline float strtof (const char* str, char** endptr){
  return strtod(str,endptr);
}
#endif

#ifdef _MSC_VER

#define snprintf c99_snprintf

int c99_snprintf(char* str, size_t size, const char* format, ...);
int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap);

inline int c99_snprintf(char* str, size_t size, const char* format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(str, size, format, ap);
    va_end(ap);

    return count;
}

inline int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

#endif // _MSC_VER

#if defined WIN32 || defined _WIN32 || defined WINCE
# include <windows.h>
const char dir_separators[] = "/\\";
const char native_separator = '\\';
namespace
{
  struct dirent
  {
	const char* d_name;
  };
  struct DIR
  {
#ifdef HAVE_WINRT
	WIN32_FIND_DATAW data;
#else
	WIN32_FIND_DATA data;
#endif
	HANDLE handle;
	dirent ent;
#ifdef HAVE_WINRT
	DIR() {};
	~DIR()
	{
	  if (ent.d_name)
		delete[] ent.d_name;
	}
#endif
  };
  inline DIR* opendir(const char* path)
  {
	DIR* dir = new DIR;
	dir->ent.d_name = 0;
#ifdef HAVE_WINRT
	cv::String full_path = cv::String(path) + "\\*";
	wchar_t wfull_path[MAX_PATH];
	size_t copied = mbstowcs(wfull_path, full_path.c_str(), MAX_PATH);
	CV_Assert((copied != MAX_PATH) && (copied != (size_t)-1));
	dir->handle = ::FindFirstFileExW(wfull_path, FindExInfoStandard,
									 &dir->data, FindExSearchNameMatch, NULL, 0);
#else
  char path_str[1024]={0,};sprintf(path_str,"%s\\*",path);
  dir->handle = ::FindFirstFileExA(path_str,FindExInfoStandard, &dir->data, FindExSearchNameMatch, NULL, 0);
#endif
	if(dir->handle == INVALID_HANDLE_VALUE)
	{
	  delete dir; // closedir will do all cleanup
	  return 0;
	}
	return dir;
  }
  inline dirent* readdir(DIR* dir)
  {
#ifdef HAVE_WINRT
	if (dir->ent.d_name != 0)
	{
	  if (::FindNextFileW(dir->handle, &dir->data) != TRUE)
		return 0;
	}
	size_t asize = wcstombs(NULL, dir->data.cFileName, 0);
	CV_Assert((asize != 0) && (asize != (size_t)-1));
	char* aname = new char[asize+1];
	aname[asize] = 0;
	wcstombs(aname, dir->data.cFileName, asize);
	dir->ent.d_name = aname;
#else
	if (dir->ent.d_name != 0)
	{
	  if (::FindNextFileA(dir->handle, &dir->data) != TRUE)
		return 0;
	}
	dir->ent.d_name = dir->data.cFileName;
#endif
	return &dir->ent;
  }
  inline void closedir(DIR* dir)
  {
	::FindClose(dir->handle);
	delete dir;
  }
}
#else
// to prevent unknown problem in dirent.h
#undef __USE_BSD
# include <dirent.h>
# include <sys/stat.h>
const char dir_separators[] = "/";
const char native_separator = '/';
#endif

inline int isDir(char * path)
{
#if defined WIN32 || defined _WIN32 || defined WINCE
  DWORD attributes;
  BOOL status = TRUE;
  {
	WIN32_FILE_ATTRIBUTE_DATA all_attrs;
#ifdef HAVE_WINRT
	wchar_t wpath[MAX_PATH];
	size_t copied = mbstowcs(wpath, path.c_str(), MAX_PATH);
	CV_Assert((copied != MAX_PATH) && (copied != (size_t)-1));
	status = ::GetFileAttributesExW(wpath, GetFileExInfoStandard, &all_attrs);
#else
	status = ::GetFileAttributesExA(path, GetFileExInfoStandard, &all_attrs);
#endif
	attributes = all_attrs.dwFileAttributes;
  }
  return status && ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else
  struct stat stat_buf;
  if (0 != stat( path, &stat_buf)){return false;}
  int is_dir = S_ISDIR( stat_buf.st_mode);
  return is_dir != 0;
#endif
}

#if defined(_MSC_VER)
inline int is_fileNotDir(char* path) {return !isDir(path);}
inline int is_exe(char* path){return is_fileexists(path);}
inline int is_dir(char *pathname, int follow_link){return isDir(pathname);}
#else
inline bool is_fileNotDir(const char* path) { //returns false if path is a folder; requires #include <sys/stat.h>
    struct stat buf;
    stat(path, &buf);
    return !S_ISDIR(buf.st_mode);
} //is_file()

inline bool is_exe(const char* path) { //requires #include <sys/stat.h>
    struct stat buf;
    stat(path, &buf);
    return (!S_ISDIR(buf.st_mode) && (buf.st_mode & 0111) );
} //is_exe()

inline int is_dir(const char *pathname, int follow_link) {
struct stat s;
if ((NULL == pathname) || (0 == strlen(pathname)))
	return 0;
int err = stat(pathname, &s);
if(-1 == err) {
        return 0; /* does not exist */
} else {
    if(S_ISDIR(s.st_mode)) {
       return 1; /* it's a dir */
    } else {
        return 0;/* exists but is no dir */
    }
}
} //is_dir
#endif

#endif // __DCM2NII_INTERNAL_HEADER__
