#include "Motion.h"

#include <string.h>

#define	MAX_COPY_FRAME	100

CMotFrame::CMotFrame()
{
	m_ImageNum	= 0;
	m_Delay		= 0;
	m_X			= 0;
	m_Y			= 0;
	m_ImageNum2	= 0;
	m_Type		= 0;
	m_Event		= 0;

	m_SubImage1	= -1;
	m_SubX1		= 0;
	m_SubY1		= 0;
	m_SubType1	= 1;

	m_SubImage2	= -1;
	m_SubX2		= 0;
	m_SubY2		= 0;
	m_SubType2	= 2;
}

void CMotFrame::operator = ( const CMotFrame& rhs )
{
	m_ImageNum	= rhs.m_ImageNum;
	m_Delay		= rhs.m_Delay;
	m_X			= rhs.m_X;
	m_Y			= rhs.m_Y;
	m_ImageNum2	= rhs.m_ImageNum2;
	m_Type		= rhs.m_Type;

	m_SubImage1	= rhs.m_SubImage1;
	m_SubX1		= rhs.m_SubX1;
	m_SubY1		= rhs.m_SubY1;
	m_SubType1	= rhs.m_SubType1;

	m_SubImage2	= rhs.m_SubImage2;
	m_SubX2		= rhs.m_SubX2;
	m_SubY2		= rhs.m_SubY2;
	m_SubType2	= rhs.m_SubType2;

	m_Event		= rhs.m_Event;
}

CAction::CAction()
{
	m_ID		= 0;
	m_pFrame	= NULL;
	m_FrameNum	= 0;
}

CAction::~CAction()
{
	if( m_pFrame )
	{
		delete[] m_pFrame;
		m_pFrame = NULL;
	}
}

BOOL CAction::Read( CFile& File )
{
	char	Name[20];
	int		i;

	if( m_pFrame )
	{
		delete[] m_pFrame;
		m_pFrame = NULL;
	}

	File.Read( &m_ID, sizeof (int) );

	File.Read( Name, sizeof (Name) );
	m_Name.SetName( Name );

	File.Read( &m_FrameNum, sizeof (int) );

	if( m_FrameNum > 0 )
	{
		m_pFrame = new CMotFrame[ m_FrameNum ];

		for( i = 0; i < m_FrameNum; i++ )
			File.Read( &m_pFrame[i], sizeof (CMotFrame) );
	}

	return TRUE;
}

void CAction::Copy( CAction& Action )
{
	CMotFrame*	pFrame;
	int			i;

	m_ID   = Action.m_ID;
	m_Name = Action.m_Name;

	for( i = 0; i < Action.m_FrameNum; i++ )
	{
		if( Action.m_pFrame && i >= 0 && i < Action.m_FrameNum )
			pFrame = &Action.m_pFrame[i];
		else
			pFrame = NULL;

		if( m_pFrame && (unsigned int)m_FrameNum < MAX_COPY_FRAME )
		{
			m_pFrame[ m_FrameNum ] = *pFrame;
			m_FrameNum++;
		}
	}
}

CMotion::CMotion()
{
	m_Type		= 0;
	m_pAction	= NULL;
	m_ActionNum	= 0;
}

CMotion::~CMotion()
{
	if( m_pAction )
	{
		delete[] m_pAction;
		m_pAction = NULL;
	}
}

void CMotion::Release()
{
	m_Type = 0;
	m_Name.Clear();
	m_Name2.SetName( "no name" );

	if( m_pAction )
	{
		delete[] m_pAction;
		m_pAction	= NULL;
		m_ActionNum	= 0;
	}
}

BOOL CMotion::Read( CFile& File )
{
	char	Name[128];
	int		i;

	if( m_pAction )
	{
		delete[] m_pAction;
		m_pAction = NULL;
	}

	File.Read( Name, sizeof (Name) );
	m_Name.SetName( Name );

	File.Read( Name, sizeof (Name) );
	m_Name2.SetName( Name );

	m_ActionNum = 0;
	File.Read( &m_ActionNum, sizeof (int) );

	if( m_ActionNum > 0 )
	{
		m_pAction = new CAction[ m_ActionNum ];

		for( i = 0; i < m_ActionNum; i++ )
			m_pAction[i].Read( File );
	}

	if( g_nLoadThread )
		g_nLoadedSprite++;

	return TRUE;
}

BOOL CMotion::LoadFromFile( const char* pFileName )
{
	CFile	File;
	BOOL	bResult;

	if( !File.Open( pFileName, FILE_READ ) )
		return FALSE;

	m_Name.SetName( pFileName );

	bResult = Read( File );
	return bResult;
}

int CMotion::GetFrameNum( int nAction )
{
	CAction*	pAction;

	if( nAction < 0 || nAction >= m_ActionNum )
		return 0;

	pAction = m_pAction + nAction;
	if( !pAction )
		return 0;

	return pAction->m_FrameNum;
}

BOOL CMotion::AddMotion( CMotion& Motion )
{
	CAction*	pAction;
	int			i;

	Release();

	m_Type = Motion.m_Type;
	m_Name.SetName( Motion.m_Name.m_Name );

	for( i = 0; i < Motion.m_ActionNum; i++ )
	{
		if( i < 0 || i >= Motion.m_ActionNum )
			continue;

		pAction = &Motion.m_pAction[i];
		if( !pAction )
			continue;

		if( m_pAction )
		{
			m_pAction[ m_ActionNum ].Copy( *pAction );
			m_ActionNum++;
		}
	}

	return TRUE;
}
