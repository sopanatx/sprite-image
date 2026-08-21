#include "Sprite.h"

#include <string.h>

static const char*	g_pTitle = "...";

#define	MAX_BOX_NUM		10

CBox::CBox()
{
	m_nLeft   = 0;
	m_nRight  = 0;
	m_nTop    = 0;
	m_nBottom = 0;
}

CFrame::CFrame()
{
	m_nIndex		= 0;
	m_pBox			= NULL;
	m_pAtkBox		= NULL;
	m_nBoxNum		= 0;
	m_nAtkBoxNum	= 0;
	m_pImage		= NULL;
}

CFrame::~CFrame()
{
	if( m_pBox )
	{
		delete[] m_pBox;
		m_pBox = NULL;
	}

	if( m_pAtkBox )
	{
		delete[] m_pAtkBox;
		m_pAtkBox = NULL;
	}

	if( m_pImage )
	{
		delete m_pImage;
		m_pImage = NULL;
	}
}

BOOL CFrame::Read( CFile& File, BOOL bCreateSurface )
{
	U8		bHasImage;
	int		i;

	if( m_pBox )
	{
		delete[] m_pBox;
		m_pBox = NULL;
	}
	m_nBoxNum = 0;

	if( m_pAtkBox )
	{
		delete[] m_pAtkBox;
		m_pAtkBox = NULL;
	}
	m_nAtkBoxNum = 0;

	File.Read( &m_nIndex,  sizeof (int) );
	File.Read( &m_nBoxNum, sizeof (int) );

	if( (U32)m_nBoxNum > MAX_BOX_NUM )
	{
		if( GetOption( 3 ) )
		{
			MessageBox( NULL, File.m_FileName,   g_pTitle, 0 );
			MessageBox( NULL, "충돌박스가 너무 많아요..",  g_pTitle, 0 );
		}
		m_nBoxNum = 0;
	}

	if( m_nBoxNum > 0 )
	{
		m_pBox = new CBox[ m_nBoxNum ];

		for( i = 0; i < m_nBoxNum; i++ )
		{
			File.Read( &m_pBox[i].m_nLeft,   sizeof (int) );
			File.Read( &m_pBox[i].m_nTop,    sizeof (int) );
			File.Read( &m_pBox[i].m_nRight,  sizeof (int) );
			File.Read( &m_pBox[i].m_nBottom, sizeof (int) );
		}
	}

	File.Read( &m_nAtkBoxNum, sizeof (int) );

	if( (U32)m_nAtkBoxNum > MAX_BOX_NUM )
	{
		if( GetOption( 3 ) )
		{
			MessageBox( NULL, File.m_FileName,  g_pTitle, 0 );
			MessageBox( NULL, "충돌박스가 너무 많아요..", g_pTitle, 0 );
		}
		m_nAtkBoxNum = 0;
	}

	if( m_nAtkBoxNum > 0 )
	{
		m_pAtkBox = new CBox[ m_nAtkBoxNum ];

		for( i = 0; i < m_nAtkBoxNum; i++ )
		{
			File.Read( &m_pAtkBox[i].m_nLeft,   sizeof (int) );
			File.Read( &m_pAtkBox[i].m_nTop,    sizeof (int) );
			File.Read( &m_pAtkBox[i].m_nRight,  sizeof (int) );
			File.Read( &m_pAtkBox[i].m_nBottom, sizeof (int) );
		}
	}

	if( m_pImage )
	{
		delete m_pImage;
		m_pImage = NULL;
	}

	bHasImage = 0;
	File.Read( &bHasImage, sizeof (U8) );

	if( !bHasImage )
		return FALSE;

	m_pImage = new CImage;

	return m_pImage->Read( File, bCreateSurface );
}

CSprite::CSprite()
{
	m_bCreateSurface = 0;
	m_nFrameNum      = 0;
	m_pFrame         = NULL;
}

CSprite::~CSprite()
{
	if( m_pFrame )
	{
		delete[] m_pFrame;
		m_pFrame = NULL;
	}
}

BOOL CSprite::Read( CFile& File, BOOL bCreateSurface )
{
	char	Name[128];
	int		i;

	if( m_pFrame )
	{
		delete[] m_pFrame;
		m_pFrame = NULL;
	}

	m_bCreateSurface = (BYTE)( bCreateSurface != 0 );

	m_Name.SetName( File.m_FileName );

	File.Read( Name, sizeof (Name) );
	m_Name1.SetName( Name );

	File.Read( Name, sizeof (Name) );
	m_Name2.SetName( Name );

	m_nFrameNum = 0;
	File.Read( &m_nFrameNum, sizeof (int) );

	if( m_nFrameNum > 0 )
	{
		m_pFrame = new CFrame[ m_nFrameNum ];

		for( i = 0; i < m_nFrameNum; i++ )
		{
			if( g_nLoadThread )
			{
				if( g_bDDraw )	g_LoadLock.Lock();
				m_pFrame[i].Read( File, m_bCreateSurface );
				if( g_bDDraw )	g_LoadLock.Unlock();
			}
			else if( g_nMapLoadThread )
			{
				if( g_bDDraw )	g_MapLoadLock.Lock();
				m_pFrame[i].Read( File, m_bCreateSurface );
				if( g_bDDraw )	g_MapLoadLock.Unlock();
			}
			else
			{
				m_pFrame[i].Read( File, m_bCreateSurface );
			}
		}
	}

	if( g_nLoadThread )
		g_nLoadedSprite++;

	return TRUE;
}
