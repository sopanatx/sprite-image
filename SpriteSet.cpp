#include "SpriteSet.h"

#include <string.h>

CSpriteSet::CSpriteSet()
{
	m_nSpriteNum = 0;
	m_pOffset    = NULL;
	m_bHasID     = FALSE;
}

CSpriteSet::~CSpriteSet()
{
	Close();
}

void CSpriteSet::Release()
{
	if( m_pOffset )
	{
		delete[] m_pOffset;
		m_pOffset = NULL;
	}

	m_nSpriteNum = 0;
	m_bHasID     = FALSE;
}

void CSpriteSet::Close()
{
	Release();

	if( m_File.m_Mode != FILE_CLOSE )
	{
		m_File.Close();
		m_File.m_Mode  = FILE_CLOSE;
		m_File.m_hFile = -1;
	}
}

BOOL CSpriteSet::Open( const char* pFileName )
{
	U8	cFirst;

	Close();

	m_Name.SetName( pFileName );

	if( !m_File.Open( pFileName, FILE_READ ) )
		return FALSE;

	m_nSpriteNum = 0;
	if( !m_File.Read( &m_nSpriteNum, sizeof (int) ) )
	{
		Close();
		return FALSE;
	}

	if( m_nSpriteNum <= 0 || m_nSpriteNum > MAX_SPRITE_SET )
	{
		Close();
		return FALSE;
	}

	m_pOffset = new __int64[ m_nSpriteNum ];

	if( !m_File.Read( m_pOffset, m_nSpriteNum * sizeof (__int64) ) )
	{
		Close();
		return FALSE;
	}

	if( m_pOffset[0] != (__int64)( 4 + 8 * m_nSpriteNum ) )
	{
		Close();
		return FALSE;
	}

	m_File.Seek( m_pOffset[0], SEEK_SET );

	if( !m_File.Read( &cFirst, sizeof (U8) ) )
	{
		Close();
		return FALSE;
	}

	m_bHasID = ( cFirst == 0 );

	return TRUE;
}

BOOL CSpriteSet::ReadSprite( int nIndex, CSprite* pSprite, BOOL bCreateSurface )
{
	int	nZero;
	int	nIDNum;

	if( !m_pOffset || !pSprite || nIndex < 0 || nIndex >= m_nSpriteNum )
		return FALSE;

	m_File.Seek( m_pOffset[ nIndex ], SEEK_SET );

	if( m_bHasID )
	{
		if( !m_File.Read( &nZero,  sizeof (int) ) )	return FALSE;
		if( !m_File.Read( &nIDNum, sizeof (int) ) )	return FALSE;

		if( nIDNum < 0 || nIDNum > MAX_SPRITE_SET_ID )
			return FALSE;

		m_File.Seek( 4 * nIDNum, SEEK_CUR );
	}

	return pSprite->Read( m_File, bCreateSurface );
}

int CSpriteSet::ReadID( int nIndex, int* pID, int nMax )
{
	int	nZero;
	int	nIDNum;
	int	i;

	if( !m_pOffset || !m_bHasID || nIndex < 0 || nIndex >= m_nSpriteNum )
		return 0;

	m_File.Seek( m_pOffset[ nIndex ], SEEK_SET );

	if( !m_File.Read( &nZero,  sizeof (int) ) )	return 0;
	if( !m_File.Read( &nIDNum, sizeof (int) ) )	return 0;

	if( nIDNum < 0 || nIDNum > MAX_SPRITE_SET_ID )
		return 0;

	for( i = 0; i < nIDNum; i++ )
	{
		int	nID;

		if( !m_File.Read( &nID, sizeof (int) ) )
			return i;

		if( pID && i < nMax )
			pID[i] = nID;
	}

	return nIDNum;
}

__int64 CSpriteSet::GetOffset( int nIndex )
{
	if( !m_pOffset || nIndex < 0 || nIndex >= m_nSpriteNum )
		return -1;

	return m_pOffset[ nIndex ];
}
