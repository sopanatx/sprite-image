#ifndef	__MOTION_H__
#define	__MOTION_H__

#include "Define.h"
#include "Name.h"
#include "File.h"

class	CMotFrame
{
public:
	short	m_ImageNum;
	short	m_Delay;
	short	m_X;
	short	m_Y;
	short	m_ImageNum2;
	short	m_Type;

	short	m_SubImage1;
	short	m_SubX1;
	short	m_SubY1;
	short	m_SubType1;

	short	m_SubImage2;
	short	m_SubX2;
	short	m_SubY2;
	short	m_SubType2;

	short	m_Event;

			CMotFrame();
	void	operator = ( const CMotFrame& rhs );
};

class	CAction
{
public:
	int			m_ID;
	CName		m_Name;
	CMotFrame*	m_pFrame;
	int			m_FrameNum;

				CAction();
				~CAction();

	BOOL		Read( CFile& File );
	void		Copy( CAction& Action );
};

class	CMotion
{
public:
	CName		m_Name;
	CName		m_Name2;
	int			m_Type;

	CAction*	m_pAction;
	int			m_ActionNum;

				CMotion();
				~CMotion();

	void		Release();
	BOOL		Read( CFile& File );
	BOOL		LoadFromFile( const char* pFileName );
	int			GetFrameNum( int nAction );
	BOOL		AddMotion( CMotion& Motion );
};

#endif
