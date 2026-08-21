#ifndef	__MOTIONSET_H__
#define	__MOTIONSET_H__

#include "Define.h"
#include "File.h"
#include "Motion.h"

enum
{
	CMO_VARIANT_PLAIN	= 0,
	CMO_VARIANT_ID		= 1
};

class	CMotionSet
{
public:
	int			m_nVariant;
	int			m_nMotionNum;
	CMotion*	m_pMotion;
	int*		m_pID;

				CMotionSet();
				~CMotionSet();

	void		Release();

	BOOL		LoadFromFile( const char* pFileName );

	CMotion*	GetMotion( int nIndex );
	int			GetID( int nIndex );

private:
	BOOL		ReadEntries( CFile& File, int nVariant );
};

#endif
