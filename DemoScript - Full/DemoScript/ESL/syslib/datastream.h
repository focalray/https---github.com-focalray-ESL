
///******************* LEGACY CODE: DO NOT EDIT ********************///

/*
	DataStream V1R1, (c) 2000-2007 lollab. All rights reserved.
	Jinhong Kim
	windgram@lollab.com

	+++ VisualStudio 2005 compliant code +++

	Revision History:
	UPDATE DATE		WHO			ACT			DESCRIPTION
	----------------------------------------------------------------------------------------------------------
	2005-Sep-21		JHKIM		change		GetRawPtr -> GetRawBufferPtr
	2005-Sep-21		JHKIM		add			GetRawPtr ( to see into the buffer directly )
	2005-Dec-16		JHKIM		fix			fixed: a bug of InitDirectMemoryAccess()
	2006-Oct-12		JHKIM		add			OpenToReadStream()
	2006-Oct-12		JHKIM		rename		GetRawPtr()  -->  GetRawCurPtr()
	2006-Oct-20		JHKIM		fix			EOF bug from GetXXXX series.
	2006-Nov-21		JHKIM		removal		removal of PutString() for security reason.
*/



#ifndef _DATASTREAM_H
#define _DATASTREAM_H



#include <windows.h>

#define DI8_MAX				(DI8)0xFF
#define DI16_MAX			(DI16)0xFFFF
#define DI32_MAX			(DI32)0xFFFFFFFF
#define DI64_MAX			(DI64)0xFFFFFFFFFFFFFFFF

typedef unsigned __int8		DI8;
typedef unsigned __int16	DI16;
typedef unsigned __int32	DI32;
typedef unsigned __int64	DI64;

typedef float				DF32;
typedef double				DF64;
typedef long double			DF80;


class DataStream
{
private:
	char		file_name[256];
	HANDLE		hFile;
	DI16		err;
	DI32		file_size;
	DI8*		buf_ptr;
	DI8*		buf_cur_ptr;
	DI8*		buf_last_ptr;
	BOOL		bDMA;
	BOOL		bOpenToRead;
	BOOL		bOpenToReadStream;

public:
	DataStream();
	DataStream(char *fn);
	~DataStream();

	
	void Init(char *fname);
	void InitDirectMemoryAccess( char* buf, DWORD speclen );
	DWORD MemoryToFile( DWORD len );

	BOOL OpenToReadStream();
	BOOL OpenToRead();
	BOOL OpenToWrite();
	BOOL CreateToWrite();
	void Close();


	template<typename T>
	DWORD GetData( T* v, int len=1 )
	{
		if ( bOpenToReadStream )
		{
			DWORD readbytes = 0;
			ReadFile(hFile, v, sizeof(T) * len, &readbytes, NULL);
			return readbytes;
		}

		for (int i=0; i<len; i++)
		{
			if (buf_cur_ptr > buf_last_ptr /*&& !bDMA*/)
			{
				err = 1;
				//*v = NULL;
				return 0;
			}
			*(v+i) = *(T*)buf_cur_ptr;
			buf_cur_ptr += sizeof(T);
		}

		return sizeof(T)*len;
	}


	DWORD GetBlock( void* v, int len )
	{
		if ( bOpenToReadStream )
		{
			DWORD readbytes = 0;
			ReadFile(hFile, v, len, &readbytes, NULL);
			return readbytes;
		}

		if (buf_cur_ptr > buf_last_ptr /*&& !bDMA*/)
		{
			err = 1;
			//*v = NULL;
			return 0;
		}

		memcpy( v, buf_cur_ptr, len );
		buf_cur_ptr += len;

		return len;
	}


	template<typename T>
	void PutData( T* v, int len=1 )
	{
		if (bDMA)
		{
			for (int i=0; i<len; i++)
			{
				*(T*)buf_cur_ptr = *(v+i);
				buf_cur_ptr += sizeof(T);
			}
		}
		else
		{
			DWORD written = 0;
			WriteFile( hFile, (void*)v, sizeof(T) * len, &written, NULL );
		}
	}

	void PutBlock( void* v, int len )
	{
		if (bDMA)
		{
			CopyMemory( (void*)buf_cur_ptr, v, len );
			buf_cur_ptr += len;
		}
		else
		{
			DWORD written;
			WriteFile(hFile, (void*)v, len, &written, NULL);
		}
	}



	void Seek( int pos );
	void Current( int* pos );
	void Skip( int len );
	void SkipBack( int len );
	void Rewind();


	// This does not make any error issue and not increase memory pointer.
	template<typename T>
	T* GetRawCurPtr()
	{
		if ( FALSE == bOpenToRead )
			return NULL;

		if (buf_cur_ptr > buf_last_ptr /*&& !bDMA*/)
			return NULL;

		return (T*)buf_cur_ptr;
	}

	template<typename T>
	T* GetRawBufferPtr()
	{
		return (T*)buf_ptr;
	}

	DI16 Err();
	DI32 GetSize();
	BOOL IsEnd();
	DWORD GetWrittenBytes();
};



#endif

