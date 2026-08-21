#include "File.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

CFile::CFile()
{
	m_hFile	 = -1;
	m_Mode	 = FILE_CLOSE;
	m_Length = 0;
}

CFile::~CFile()
{
	if( m_Mode != FILE_CLOSE )
		_close( m_hFile );
}

BOOL CFile::Open( const char* pFileName, int Mode )
{
	char	FileName[256];

	memset( FileName, 0, sizeof (FileName) );
	strcpy( FileName, pFileName );
	strcpy( m_FileName, pFileName );

	switch( Mode )
	{
	case FILE_READ:
		m_hFile = _open( FileName, _O_RDONLY | _O_BINARY, _S_IREAD | _S_IWRITE );
		if( m_hFile == -1 )	break;
		m_Mode = FILE_READ;
		return TRUE;

	case FILE_WRITE:
		m_hFile = _open( FileName, _O_WRONLY | _O_CREAT | _O_BINARY, _S_IREAD | _S_IWRITE );
		if( m_hFile == -1 )	break;
		m_Mode = FILE_WRITE;
		return TRUE;

	case FILE_READ_TEXT:
		m_hFile = _open( FileName, _O_RDONLY | _O_TEXT, _S_IREAD | _S_IWRITE );
		if( m_hFile == -1 )	break;
		m_Mode = FILE_READ_TEXT;
		return TRUE;

	case FILE_WRITE_TEXT:
		m_hFile = _open( FileName, _O_WRONLY | _O_TEXT, _S_IREAD | _S_IWRITE );
		if( m_hFile == -1 )	break;
		m_Mode = FILE_WRITE_TEXT;
		return TRUE;
	}

	m_Mode = FILE_CLOSE;
	return FALSE;
}

BOOL CFile::Close()
{
	if( m_Mode == FILE_CLOSE )
		return FALSE;

	_close( m_hFile );
	return TRUE;
}

BOOL CFile::Read( void* pBuf, unsigned int Size )
{
	if( m_Mode == FILE_CLOSE || m_Mode == FILE_WRITE )
		return FALSE;

	return ( _read( m_hFile, pBuf, Size ) > 0 );
}

BOOL CFile::Write( void* pBuf, unsigned int Size )
{
	if( m_Mode == FILE_CLOSE || m_Mode == FILE_READ )
		return FALSE;

	return ( _write( m_hFile, pBuf, Size ) != -1 );
}

BOOL CFile::ReadLine( char* pBuffer )
{
	char	Line[256];
	char*	pWrite;
	char	c;

	memset( Line, 0, sizeof (Line) );
	pWrite = Line;

	while( 1 )
	{
		if( m_Mode == FILE_CLOSE || m_Mode == FILE_WRITE )
			return FALSE;

		if( _read( m_hFile, &c, 1 ) <= 0 )
			return FALSE;

		if( c == 0 || c == '\n' )
			break;

		if( c != '\r' && c != 0xFEFF )
			*pWrite++ = c;
	}

	sprintf( pBuffer, "%s", Line );
	return TRUE;
}

__int64 CFile::Seek( __int64 Offset, int Origin )
{
	return _lseeki64( m_hFile, Offset, Origin );
}
