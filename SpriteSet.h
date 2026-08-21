#ifndef	__SPRITESET_H__
#define	__SPRITESET_H__

#include "Define.h"
#include "Name.h"
#include "File.h"
#include "Sprite.h"

#define	MAX_SPRITE_SET		20000
#define	MAX_SPRITE_SET_ID	64

class	CSpriteSet
{
public:
	CFile		m_File;
	CName		m_Name;
	int			m_nSpriteNum;
	__int64*	m_pOffset;
	BOOL		m_bHasID;

				CSpriteSet();
				~CSpriteSet();

	void		Release();

	BOOL		Open( const char* pFileName );
	void		Close();

	BOOL		ReadSprite( int nIndex, CSprite* pSprite, BOOL bCreateSurface );

	int			ReadID( int nIndex, int* pID, int nMax );

	__int64		GetOffset( int nIndex );
};

#endif
