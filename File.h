#ifndef	__FILE_H__
#define	__FILE_H__

#include "Define.h"

class	CFile
{
public:
	int		m_hFile;
	int		m_Mode;
	char	m_FileName[256];
	int		m_Length;

			CFile();
			~CFile();

	BOOL	Open( const char* pFileName, int Mode );
	BOOL	Close();

	BOOL	Read ( void* pBuf, unsigned int Size );
	BOOL	Write( void* pBuf, unsigned int Size );
	BOOL	ReadLine( char* pBuffer );

	__int64	Seek( __int64 Offset, int Origin );
};

#endif
