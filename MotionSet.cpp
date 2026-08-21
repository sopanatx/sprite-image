#include "MotionSet.h"

#include <string.h>

CMotionSet::CMotionSet()
{
	m_nVariant   = CMO_VARIANT_PLAIN;
	m_nMotionNum = 0;
	m_pMotion    = NULL;
	m_pID        = NULL;
}

CMotionSet::~CMotionSet()
{
	Release();
}

void CMotionSet::Release()
{
	if( m_pMotion )
	{
		delete[] m_pMotion;
		m_pMotion = NULL;
	}

	if( m_pID )
	{
		delete[] m_pID;
		m_pID = NULL;
	}

	m_nMotionNum = 0;
	m_nVariant   = CMO_VARIANT_PLAIN;
}

BOOL CMotionSet::ReadEntries( CFile& File, int nVariant )
{
	int	nUnknown0, nUnknown1;
	int	i;

	m_pMotion = new CMotion[ m_nMotionNum ];

	if( nVariant == CMO_VARIANT_ID )
		m_pID = new int[ m_nMotionNum ];

	for( i = 0; i < m_nMotionNum; i++ )
	{
		if( nVariant == CMO_VARIANT_ID )
		{
			if( !File.Read( &nUnknown0, sizeof (int) ) )	return FALSE;
			if( !File.Read( &nUnknown1, sizeof (int) ) )	return FALSE;
			if( !File.Read( &m_pID[i],  sizeof (int) ) )	return FALSE;
		}

		if( !m_pMotion[i].Read( File ) )
			return FALSE;

		if( m_pMotion[i].m_Name.m_Name[0] == 0 )
			return FALSE;
	}

	return TRUE;
}

BOOL CMotionSet::LoadFromFile( const char* pFileName )
{
	CFile		File;
	__int64		nSize;
	int			nVariant;

	Release();

	if( !File.Open( pFileName, FILE_READ ) )
		return FALSE;

	nSize = File.Seek( 0, SEEK_END );
	File.Seek( 0, SEEK_SET );

	for( nVariant = CMO_VARIANT_PLAIN; nVariant <= CMO_VARIANT_ID; nVariant++ )
	{
		BOOL	bOK;

		File.Seek( 0, SEEK_SET );

		m_nMotionNum = 0;
		if( !File.Read( &m_nMotionNum, sizeof (int) ) )
			break;

		if( m_nMotionNum <= 0 || m_nMotionNum > 5000 )
			break;

		bOK = ReadEntries( File, nVariant );

		if( bOK && File.Seek( 0, SEEK_CUR ) == nSize )
		{
			m_nVariant = nVariant;

			File.Close();
			File.m_Mode = FILE_CLOSE;
			return TRUE;
		}

		Release();
	}

	File.Close();
	File.m_Mode = FILE_CLOSE;
	Release();
	return FALSE;
}

CMotion* CMotionSet::GetMotion( int nIndex )
{
	if( !m_pMotion || nIndex < 0 || nIndex >= m_nMotionNum )
		return NULL;

	return &m_pMotion[ nIndex ];
}

int CMotionSet::GetID( int nIndex )
{
	if( !m_pID || nIndex < 0 || nIndex >= m_nMotionNum )
		return -1;

	return m_pID[ nIndex ];
}
