#ifndef SKY_SDFS_SDK_H
#define SKY_SDFS_SDK_H

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

#ifdef   __cplusplus 
extern "C" {
#endif

typedef struct fileinfo{
	long long 	fileid;
	char 		name[100];
	int			filetype;
	int 		filemode;
	long long 	beginTime;
	int 		owner;
	int			copysize;
	int 		blocksize;
	long long 	link;
	int 		blocklength;
	}fileinfo;

SKY_SDFS_API(int) sky_sdfs_init(char* path);
SKY_SDFS_API(int) sky_sdfs_cleanup(void);

SKY_SDFS_API(long long)  sky_sdfs_createfile(const char* filename , int blocklength , int copysize);
SKY_SDFS_API(int) sky_sdfs_openfile(long long fileid , int mode);
SKY_SDFS_API(void) sky_sdfs_close( int fd);
SKY_SDFS_API(int) sky_sdfs_write( int fd,const void* buf,int nbytes);
SKY_SDFS_API(int) sky_sdfs_read( int fd , const void* buf , int nbytes);
SKY_SDFS_API(int) getlasterror( int fd , void* errorinfo,int len);
SKY_SDFS_API(long long) sky_sdfs_lseek( int fd, long long offset, int whence);

SKY_SDFS_API(int) sky_sdfs_fileinfo( int fd , fileinfo* info);

#ifdef   __cplusplus 
}
#endif

#endif
