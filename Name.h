#ifndef	__NAME_H__
#define	__NAME_H__

#include "Define.h"

class	CName
{
public:
	char	m_Name[160];
	int		m_Reserved0;
	int		m_Reserved1;

			CName();
			~CName();

	void	SetName( const char* pName );
	void	CutPath();
	void	Clear();

	void	operator = ( const CName& rhs );
};

#endif
