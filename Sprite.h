#ifndef	__SPRITE_H__
#define	__SPRITE_H__

#include "Define.h"
#include "Name.h"
#include "File.h"
#include "Image.h"

class	CBox
{
public:
	int		m_nLeft;
	int		m_nRight;
	int		m_nTop;
	int		m_nBottom;

			CBox();
};

class	CFrame
{
public:
	int		m_nIndex;
	CBox*	m_pBox;
	CBox*	m_pAtkBox;
	int		m_nBoxNum;
	int		m_nAtkBoxNum;
	CImage*	m_pImage;

			CFrame();
			~CFrame();

	BOOL	Read( CFile& File, BOOL bCreateSurface );
};

class	CSprite
{
public:
	BYTE	m_bCreateSurface;
	CName	m_Name;
	CName	m_Name1;
	CName	m_Name2;
	int		m_nFrameNum;
	CFrame*	m_pFrame;

			CSprite();
			~CSprite();

	BOOL	Read( CFile& File, BOOL bCreateSurface );
};

#endif
