#include "Name.h"

#include <stdio.h>
#include <string.h>

CName::CName()
{
	memset( this, 0, sizeof (m_Name) );
}

CName::~CName()
{
}

void CName::SetName( const char* pName )
{
	memset( m_Name, 0, sizeof (m_Name) );

	if( pName )
		sprintf( m_Name, "%s", pName );
}

void CName::CutPath()
{
	char	Temp[160];
	int		i;

	for( i = (int)strlen( m_Name ) - 1; i >= 0; i-- )
	{
		if( m_Name[i] == '/' || m_Name[i] == '\\' )
		{
			sprintf( Temp, "%s", &m_Name[i + 1] );
			sprintf( m_Name, "%s", Temp );
			return;
		}
	}
}

void CName::Clear()
{
	memset( m_Name, 0, sizeof (m_Name) );
}

void CName::operator = ( const CName& rhs )
{
	sprintf( m_Name, rhs.m_Name );
}
