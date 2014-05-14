//#include <stdio.h>
#ifndef SKY_SDFS_SDK_H
#define SKY_SDFS_SDK_H

#include <stdio.h>

#ifndef WIN32
#define SKY_SDFS_API(X) X 

#else

#ifdef SKYFSCLIENTC_EXPORTS
#define SKY_SDFS_API(type) extern"C" __declspec(dllexport) type __stdcall 
#else
#define SKY_SDFS_API(type) extern"C" __declspec(dllimport) type __stdcall
#endif

#endif


#define TRUE 		1
#define FALSE 		0

#define O_CREATE		0
#define O_READ			1
#define O_WRITE			2

#define SDFS_SEEK_SET 0
#define SDFS_SEEK_CUR 1
#define SDFS_SEEK_END 2

#define NORMAL_FILE		0
#define INDEX_FILE		1
#define LITTLE_FILE		2

#define STARTTIME		0
#define ENDTIME			1


#ifdef   __cplusplus 
extern "C" {
#endif

typedef struct fileinfo{
	long long 	fileid;
	char 		name[100];
	int			filetype;
	int 		filemode;
	char	 	beginTime[23];
	int 		owner;
	int			copysize;
	int 		blocksize;
	long long 	link;
	int 		blocklength;
}fileinfo;

typedef struct clientconfig{
	char		cn_ips[100];
	int			cnport;
	int			indexport;
	int			snport;
	char		rack[100];
}clientconfig;

SKY_SDFS_API(int) sky_sdfs_init(const char* path);

SKY_SDFS_API(int) sky_sdfs_init_ex(const char* path,int paras,...); //sky_sdfs_init_ex("config.ini",2,"cnips","192.168.8.101","cnport",29001);
SKY_SDFS_API(int) sky_sdfs_init_list(const char* path,int paras , va_list args);

SKY_SDFS_API(int) client_uninit(void);
SKY_SDFS_API(int) client_init(clientconfig* config);

SKY_SDFS_API(long long) client_create(fileinfo* info);
SKY_SDFS_API(int) client_open(long long fileid , int mode);

SKY_SDFS_API(long long) client_upload(fileinfo* info , const char* filepath);
SKY_SDFS_API(long long) client_download(long long fileid , const char* filepath);

SKY_SDFS_API(int) client_write( int fd,const void* buf,int nbytes);
SKY_SDFS_API(int) client_read( int fd , const void* buf , int nbytes);

SKY_SDFS_API(int) getlasterror( int fd , void* errorinfo,int len);
SKY_SDFS_API(void) client_close( int fd);

SKY_SDFS_API(long long) client_delete(long long fileid);
SKY_SDFS_API(int) client_lock(long long fileid);
SKY_SDFS_API(int) client_unlock(long long fileid);

SKY_SDFS_API(long long) client_search(int fd , const char* time , int mark);
SKY_SDFS_API(long long) client_postion( int fd, long long offset, int whence);

SKY_SDFS_API(int) get_fileinfo( long long fileid , fileinfo* info);

SKY_SDFS_API(int) sky_sdfs_sync(int fd);
SKY_SDFS_API(int) sky_sdfs_close_pipe(void);

#ifdef   __cplusplus 
}
#endif

#endif
