#include "Image.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

CImage::CImage()
{
	m_ColorType			= COLOR_TYPE_RGB565;
	m_TexBlendType		= TEX_BLEND_NORMAL;
	m_FileType			= FILE_TYPE_TGA;

	m_SearchColor.r		= 255;
	m_SearchColor.g		= 0;
	m_SearchColor.b		= 255;

	m_ClipRect.left		= 0;
	m_ClipRect.top		= 0;
	m_ClipRect.right	= xRight;
	m_ClipRect.bottom	= yBottom;

	m_pTexture			= NULL;
	m_pOffScreen		= NULL;

	m_Width				= 0;
	m_Height			= 0;
	m_DrawWidth			= 0;
	m_DrawHeight		= 0;
	m_StartX			= 0;
	m_StartY			= 0;
	m_NumPixels			= 0;
}

CImage::~CImage()
{
	if( m_pTexture )
	{
		g_nTextureCount--;
		m_pTexture->Release();
	}
	m_pTexture = NULL;

	if( m_pOffScreen )
	{
		m_pOffScreen->Release();
		m_pOffScreen = NULL;
	}
}

void CImage::Release()
{
	m_ColorType			= COLOR_TYPE_RGB565;
	m_TexBlendType		= TEX_BLEND_NORMAL;
	m_FileType			= FILE_TYPE_TGA;

	m_SearchColor.r		= 255;
	m_SearchColor.g		= 0;
	m_SearchColor.b		= 255;

	m_ClipRect.left		= 0;
	m_ClipRect.top		= 0;
	m_ClipRect.right	= xRight;
	m_ClipRect.bottom	= yBottom;

	m_Width				= 0;
	m_Height			= 0;
	m_DrawWidth			= 0;
	m_DrawHeight		= 0;
	m_StartX			= 0;
	m_StartY			= 0;
	m_NumPixels			= 0;

	if( m_pTexture )
	{
		g_nTextureCount--;
		m_pTexture->Release();
		m_pTexture = NULL;
	}

	if( m_pOffScreen )
	{
		m_pOffScreen->Release();
		m_pOffScreen = NULL;
	}
}

void CImage::SkipImage( CFile& File, int nNumPixels, int nColorType, U32 nDataSize )
{
	if( nColorType == COLOR_TYPE_A8R8G8B8 )
	{
		File.Seek( nNumPixels * sizeof (U32), SEEK_CUR );
		return;
	}

	if( nColorType == COLOR_TYPE_DXT1 || nColorType == COLOR_TYPE_DXT2 ||
		nColorType == COLOR_TYPE_DXT3 || nColorType == COLOR_TYPE_DXT4 ||
		nColorType == COLOR_TYPE_DXT5 )
	{
		File.Seek( nDataSize, SEEK_CUR );
		return;
	}

	File.Seek( nNumPixels * sizeof (U16), SEEK_CUR );
}

BOOL CImage::Read( CFile& File, BOOL bCreateSurface )
{
	if( g_bDDraw )
		return ReadDDraw( File, bCreateSurface );

	return ReadD3D( File, bCreateSurface );
}

BOOL CImage::LoadFromFile( const char* pFileName )
{
	CFile	File;
	BOOL	bResult;

	SetName( pFileName );
	CutPath();

	if( !File.Open( pFileName, FILE_READ ) )
		return FALSE;

	if( g_bDDraw )
		bResult = ReadDDraw( File, TRUE );
	else
		bResult = ReadD3D( File, TRUE );

	File.Close();
	return bResult;
}

BOOL CImage::ReadDDraw( CFile& File, BOOL bCreateSurface )
{
	if( m_pOffScreen )
	{
		m_pOffScreen->Release();
		m_pOffScreen = NULL;
	}

	if( !File.Read( &m_ColorType, sizeof (int) ) )
	{
		MessageBox( NULL, "\tif( !File.Read(&m_ColorType, sizeof (int)) )실패 ", NULL, 0 );
		return FALSE;
	}

	if( !File.Read( &m_TexBlendType, sizeof (int) ) )
	{
		MessageBox( NULL, "if( !File.Read(&m_TexBlendType, sizeof (int)) ) 실패 ", NULL, 0 );
		return FALSE;
	}

	if( !File.Read( &m_FileType, sizeof (int) ) )
	{
		MessageBox( NULL, "if( !File.Read(&m_FileType, sizeof (int)) ) 실패 ", NULL, 0 );
		return FALSE;
	}

	if( !File.Read( &m_SearchColor, sizeof (RGBVal) ) )
	{
		MessageBox( NULL, "\tif( !File.Read(&m_SearchColor, sizeof(RGBVal)) ) 실패 ", NULL, 0 );
		return FALSE;
	}

	if( !File.Read( &m_Width, sizeof (int) ) )
	{
		MessageBox( NULL, "if( !File.Read(&m_Width, sizeof (int)) ) 실패 ", NULL, 0 );
		return FALSE;
	}

	if( !File.Read( &m_Height, sizeof (int) ) )
	{
		MessageBox( NULL, "if( !File.Read(&m_Height, sizeof (int)) ) 실패 ", NULL, 0 );
		return FALSE;
	}

	if( !File.Read( &m_NumPixels, sizeof (int) ) )
	{
		MessageBox( NULL, "if( !File.Read(&m_NumPixels, sizeof (int))) 실패 ", NULL, 0 );
		return FALSE;
	}

	if( m_Width == 0 || m_Height == 0 )
		return FALSE;

	if( bCreateSurface )
	{
		g_pDraw->CreateOffscreen( &m_pOffScreen, m_Width, m_Height, FALSE );

		if( !m_pOffScreen )
		{
			MessageBox( NULL,
				"g_pDraw->CreateOffscreen( &m_pOffScreen, m_Width, m_Height, FALSE );",
				"NULL", 0 );
			return FALSE;
		}

		g_pDraw->FillColor  ( m_pOffScreen, g_pDraw->MakeColor( 255, 0, 255 ) );
		g_pDraw->SetColorKey( m_pOffScreen, g_pDraw->MakeColor( 255, 0, 255 ) );
	}

	switch( m_ColorType )
	{
	case COLOR_TYPE_A1R5G5B5:
		if( bCreateSurface )
		{
			U16*	pImage = new U16[ m_NumPixels ];

			if( !File.Read( pImage, m_NumPixels * sizeof (U16) ) )
			{
				MessageBox( NULL, "\tif( !File.Read(&m_ColorType, sizeof (int)) ) 실패 ", NULL, 0 );
				return FALSE;
			}

			g_pDraw->PutImage1555( pImage, 0, 0, m_Width, m_Height,
								   m_Width, m_Height, m_pOffScreen );
			delete[] pImage;
		}
		else
		{
			File.Seek( m_NumPixels * sizeof (U16), SEEK_CUR );
		}
		break;

	case COLOR_TYPE_RGB565:
		if( bCreateSurface )
		{
			U16*	pImage = (U16*)new U32[ m_NumPixels ];

			if( !File.Read( pImage, m_NumPixels * sizeof (U16) ) )
			{
				MessageBox( NULL, "if( !File.Read( pImage, m_NumPixels * sizeof(U16)) ) 실패 ", NULL, 0 );
				return FALSE;
			}

			g_pDraw->PutImage565( pImage, 0, 0, m_Width, m_Height,
								  m_Width, m_Height, m_pOffScreen );
			delete[] pImage;
		}
		else
		{
			File.Seek( m_NumPixels * sizeof (U32), SEEK_CUR );
		}
		break;

	case COLOR_TYPE_A8R8G8B8:
		if( bCreateSurface )
		{
			U32*	pImage = new U32[ m_NumPixels ];

			File.Read( pImage, m_NumPixels * sizeof (U32) );

			g_pDraw->PutImage8888( pImage, 0, 0, m_Width, m_Height,
								   m_Width, m_Height, m_pOffScreen );
			delete[] pImage;
		}
		else
		{
			File.Seek( m_NumPixels * sizeof (U32), SEEK_CUR );
		}
		break;

	default:
		MessageBox( NULL, "픽셀포멧을 찾을수 없습니다.", "NULL", 0 );
		return FALSE;
	}

	m_DrawWidth  = m_Width;
	m_DrawHeight = m_Height;
	m_StartX     = 0;
	m_StartY     = 0;

	return TRUE;
}

BOOL CImage::ReadD3D( CFile& File, BOOL bCreateSurface )
{
	D3DFORMAT			Format;
	D3DSURFACE_DESC		Desc;
	D3DLOCKED_RECT		LockRect;
	BOOL				bSupported;
	BOOL				bCompressed;
	U32					nDataSize;
	int					i;

	if( m_pTexture )
	{
		g_nTextureCount--;
		m_pTexture->Release();
		m_pTexture = NULL;
	}

	if( !File.Read( &m_ColorType,    sizeof (int)    ) )	return FALSE;
	if( !File.Read( &m_TexBlendType, sizeof (int)    ) )	return FALSE;
	if( !File.Read( &m_FileType,     sizeof (int)    ) )	return FALSE;
	if( !File.Read( &m_SearchColor,  sizeof (RGBVal) ) )	return FALSE;
	if( !File.Read( &m_Width,        sizeof (int)    ) )	return FALSE;
	if( !File.Read( &m_Height,       sizeof (int)    ) )	return FALSE;
	if( !File.Read( &m_NumPixels,    sizeof (int)    ) )	return FALSE;

	switch( m_ColorType )
	{
	case COLOR_TYPE_A8R8G8B8:	Format = D3DFMT_A8R8G8B8;	break;
	case COLOR_TYPE_A1R5G5B5:	Format = D3DFMT_A1R5G5B5;	break;
	case COLOR_TYPE_A4R4G4B4:	Format = D3DFMT_A4R4G4B4;	break;
	case COLOR_TYPE_DXT1:		Format = D3DFMT_DXT1;		break;
	case COLOR_TYPE_DXT2:		Format = D3DFMT_DXT2;		break;
	case COLOR_TYPE_DXT3:		Format = D3DFMT_DXT3;		break;
	case COLOR_TYPE_DXT4:		Format = D3DFMT_DXT4;		break;
	case COLOR_TYPE_DXT5:		Format = D3DFMT_DXT5;		break;
	default:					Format = D3DFMT_A1R5G5B5;	break;
	}

	nDataSize  = 0;
	bSupported = g_pDevice->CheckFormat( Format );

	if( m_ColorType == COLOR_TYPE_DXT1 || m_ColorType == COLOR_TYPE_DXT2 ||
		m_ColorType == COLOR_TYPE_DXT3 || m_ColorType == COLOR_TYPE_DXT4 ||
		m_ColorType == COLOR_TYPE_DXT5 )
	{
		bCompressed = TRUE;

		if( !bSupported )
			bCreateSurface = FALSE;

		if( !File.Read( &nDataSize, sizeof (int) ) )
			return FALSE;
	}
	else
	{
		bCompressed = FALSE;
	}

	if( !bCreateSurface )
	{
		m_DrawWidth  = m_Width;
		m_DrawHeight = m_Height;

		SkipImage( File, m_NumPixels, m_ColorType, nDataSize );
		return TRUE;
	}

	if( bSupported )
	{
		if( !CreateTexture( &m_pTexture, m_Width, m_Height, Format, File, nDataSize ) )
		{
			SkipImage( File, m_NumPixels, m_ColorType, nDataSize );
			return FALSE;
		}
	}
	else
	{
		if( Format != D3DFMT_A8R8G8B8 )
		{
			SkipImage( File, m_NumPixels, m_ColorType, nDataSize );
			return FALSE;
		}

		if( !CreateTexture( &m_pTexture, m_Width, m_Height, D3DFMT_A4R4G4B4, File, nDataSize ) )
		{
			SkipImage( File, m_NumPixels, m_ColorType, nDataSize );
			return FALSE;
		}

		Format = D3DFMT_A4R4G4B4;
	}

	if( bCompressed )
	{
		m_DrawWidth  = m_Width;
		m_DrawHeight = m_Height;
		m_StartX     = 0;
		m_StartY     = 0;
		return TRUE;
	}

	m_pTexture->GetLevelDesc( 0, &Desc );

	if( Desc.Width != m_Width || Desc.Height != m_Height )
	{
		SkipImage( File, m_NumPixels, m_ColorType, nDataSize );
		return FALSE;
	}

	m_pTexture->LockRect( 0, &LockRect, NULL, 0 );

	if( m_ColorType == COLOR_TYPE_A8R8G8B8 && Format == D3DFMT_A8R8G8B8 )
	{
		if( !File.Read( LockRect.pBits, m_NumPixels * sizeof (U32) ) )
		{
			m_pTexture->UnlockRect( 0 );
			return FALSE;
		}
	}
	else if( ( m_ColorType == COLOR_TYPE_A1R5G5B5 || m_ColorType == COLOR_TYPE_RGB565 )
			 && Format == D3DFMT_A1R5G5B5 )
	{
		if( !File.Read( LockRect.pBits, m_NumPixels * sizeof (U16) ) )
		{
			m_pTexture->UnlockRect( 0 );
			return FALSE;
		}
	}
	else if( m_ColorType == COLOR_TYPE_A8R8G8B8 && Format == D3DFMT_A4R4G4B4 )
	{
		U8*		pSource = new U8 [ m_NumPixels * 4 ];
		U16*	pImage  = new U16[ m_NumPixels ];

		File.Read( pSource, m_NumPixels * sizeof (U32) );

		for( i = 0; i < m_NumPixels; i++ )
		{
			pImage[i] = (U16)(   ( pSource[i * 4 + 0] >> 4 )
							 | ( ( pSource[i * 4 + 1] >> 4 ) <<  4 )
							 | ( ( pSource[i * 4 + 2] >> 4 ) <<  8 )
							 | ( ( pSource[i * 4 + 3] >> 4 ) << 12 ) );
		}

		PutImage16( LockRect.pBits, m_Width, m_Width, m_Height,
					pImage, m_Width, m_Height, 0, 0, 0 );

		delete[] pSource;
		delete[] pImage;
	}
	else
	{
		SkipImage( File, m_NumPixels, m_ColorType, nDataSize );
		m_pTexture->UnlockRect( 0 );
		return FALSE;
	}

	m_DrawWidth  = m_Width;
	m_DrawHeight = m_Height;
	m_StartX     = 0;
	m_StartY     = 0;

	m_pTexture->UnlockRect( 0 );
	return TRUE;
}

BOOL CImage::CreateTexture( LPDIRECT3DTEXTURE9* ppTexture, int nWidth, int nHeight,
							D3DFORMAT Format, CFile& File, U32 nDataSize )
{
	D3DLOCKED_RECT	LockRect;
	HRESULT			hr;

	if( g_bDDraw )
	{
		g_pDraw->CreateOffscreen( &m_pOffScreen, nWidth, nHeight, FALSE );
		return TRUE;
	}

	if( Format == D3DFMT_DXT1 || Format == D3DFMT_DXT2 || Format == D3DFMT_DXT3 ||
		Format == D3DFMT_DXT4 || Format == D3DFMT_DXT5 )
	{
		U8*	pData = new U8[ nDataSize ];

		if( !File.Read( pData, nDataSize ) )
			return FALSE;

		hr = D3DXCreateTextureFromFileInMemoryEx(
					g_pDevice->m_pd3dDevice, pData, nDataSize,
					nWidth, nHeight, 1, 0, Format, D3DPOOL_MANAGED,
					D3DX_FILTER_LINEAR, D3DX_FILTER_NONE,
					0x00FF00FF, NULL, NULL, ppTexture );

		if( FAILED( hr ) )
		{
			OutputDebugString( "D3DXCreateTexture FAILED\n" );
			return FALSE;
		}

		g_nTextureCount++;
		delete[] pData;
	}
	else
	{
		hr = D3DXCreateTexture( g_pDevice->m_pd3dDevice, nWidth, nHeight,
								1, 0, Format, D3DPOOL_MANAGED, ppTexture );

		if( FAILED( hr ) )
		{
			if( hr == E_OUTOFMEMORY )
			{
				OutputDebugString( "OUT_OF_MEMORY\n" );
				return FALSE;
			}

			OutputDebugString( "D3DXCreateTexture FAILED\n" );
			return FALSE;
		}

		g_nTextureCount++;
	}

	(*ppTexture)->LockRect( 0, &LockRect, NULL, 0 );
	(*ppTexture)->UnlockRect( 0 );

	return TRUE;
}

void CImage::Draw( int x, int y, char nFlip, float fScaleX, float fScaleY,
				   float fAngle, U32 dwColor, float fCenterX, float fCenterY )
{
	char	nDir;
	int		nAlpha;

	if( g_bDDraw )
	{
		if( !g_pDraw || !m_pOffScreen )
			return;

		if( fAngle == 0.0f )								nDir = nFlip;
		else if( fAngle <= -270.0f || fAngle >= -90.0f )	nDir = 1;
		else												nDir = -1;

		nAlpha = dwColor >> 27;

		if( nDir == 1 )
		{
			if( m_TexBlendType == TEX_BLEND_ADD )
				g_pDraw->BltAdd  ( m_pOffScreen, 0, 0, m_Width, m_Height,
								   g_pDraw->m_pBack, x, y, m_Width, m_Height );
			else if( !GetOption( 2 ) )
				g_pDraw->Blt     ( m_pOffScreen, 0, 0, m_Width, m_Height,
								   g_pDraw->m_pBack, x, y, m_Width, m_Height, TRUE );
			else
				g_pDraw->BltAlpha( m_pOffScreen, 0, 0, m_Width, m_Height,
								   g_pDraw->m_pBack, x, y, m_Width, m_Height, nAlpha );
		}
		else
		{
			if( m_TexBlendType == TEX_BLEND_ADD )
				g_pDraw->BltAddMirror  ( m_pOffScreen, 0, 0, m_Width, m_Height,
										 g_pDraw->m_pBack, x, y, m_Width, m_Height );
			else if( !GetOption( 2 ) )
				g_pDraw->BltMirror     ( m_pOffScreen, 0, 0, m_Width, m_Height,
										 g_pDraw->m_pBack, x, y, m_Width, m_Height, TRUE );
			else
				g_pDraw->BltAlphaMirror( m_pOffScreen, 0, 0, m_Width, m_Height,
										 g_pDraw->m_pBack, x, y, m_Width, m_Height, nAlpha );
		}
		return;
	}

	if( !g_pDevice || !m_pTexture )
		return;

	if( fAngle == 0.0f )
	{
		if( (float)g_rcClip.left > m_DrawWidth  * fScaleX + x ||
			(float)g_rcClip.top  > m_DrawHeight * fScaleY + y ||
			x >= g_rcClip.right || y >= g_rcClip.bottom )
			return;
	}
	else
	{
		D3DXMATRIX	mRot, mPos, mWorld;
		float		fRad = fAngle * 0.017452778f;

		if( fCenterX >= 10000.0f || fCenterY >= 10000.0f )
		{
			double	c, s;
			float	fx, fy;

			if( fCenterY < 10000.0f )
			{
				MatrixRotationX( &mRot, fRad );
				fCenterX = fCenterX - 10000.0f;
			}
			else
			{
				MatrixRotationY( &mRot, fRad );
			}

			c  = cos( fAngle * 0.01745329251994328 );
			s  = sin( fAngle * 0.01745329251994328 );
			fx = (float)( fCenterX * c - fCenterX * s );
			fy = (float)( fx * s + fx * c );

			MatrixTranslate( &mPos, fx, fy, 0.0f );
			MatrixTranspose( &mPos, &mPos );
		}
		else
		{
			MatrixTranslate( &mPos, fCenterX, fCenterY, 0.0f );
			MatrixTranspose( &mPos, &mPos );
			MatrixRotationZ( &mRot, fRad );
		}

		MatrixTranspose( &mRot, &mRot );
		MatrixMultiply( &mWorld, &mPos, &mRot );

		g_pDevice->m_pd3dDevice->SetVertexShaderConstantF( 4, (float*)&mWorld, 4 );
	}

	g_pDevice->m_pd3dDevice->SetTexture( 0, m_pTexture );

	if( nFlip == -1 )
		DrawPrimitive( (int)( m_DrawWidth * fScaleX + x ), y,
					   -(int)( m_Width * fScaleX ), (int)( m_Height * fScaleY ),
					   0.0f, 0.0f, 1.0f, 1.0f, dwColor, 1 );
	else
		DrawPrimitive( x, y,
					   (int)( m_Width * fScaleX ), (int)( m_Height * fScaleY ),
					   0.0f, 0.0f, 1.0f, 1.0f, dwColor, 1 );

	if( fAngle != 0.0f )
	{
		D3DXMATRIX	mIdentity;

		MatrixIdentity( &mIdentity );
		g_pDevice->m_pd3dDevice->SetVertexShaderConstantF( 4, (float*)&mIdentity, 4 );
	}
}

void CImage::Draw( int x, int y, char nFlip, float fScale,
				   float fAngle, U32 dwColor, float fCenterX, float fCenterY )
{
	Draw( x, y, nFlip, fScale, fScale, fAngle, dwColor, fCenterX, fCenterY );
}

void CImage::DrawPrimitive( int x, int y, int nWidth, int nHeight,
							float fU0, float fV0, float fU1, float fV1,
							U32 dwColor, int nStage )
{
	float	vConst[8];

	if( g_bDDraw )
	{
		if( !g_pDraw || !m_pOffScreen )
			return;

		if( !GetOption( 2 ) )
			g_pDraw->Blt     ( m_pOffScreen, 0, 0, m_Width, m_Height,
							   g_pDraw->m_pBack, x, y, m_Width, m_Height, TRUE );
		else
			g_pDraw->BltAlpha( m_pOffScreen, 0, 0, m_Width, m_Height,
							   g_pDraw->m_pBack, x, y, m_Width, m_Height,
							   dwColor >> 27 );
		return;
	}

	g_pDevice->m_pd3dDevice->SetRenderState( D3DRS_TEXTUREFACTOR, dwColor );
	g_pDevice->m_pd3dDevice->SetFVF( D3DFVF_XYZ );

	if( m_TexBlendType == TEX_BLEND_ADD )
		g_pDevice->SetBlend( BLEND_ADD );
	else
		g_pDevice->SetBlend( BLEND_ALPHA );

	vConst[0] = 0.5f * nWidth  + x;
	vConst[1] = 0.5f * nHeight + y;
	vConst[2] = (float)nWidth;
	vConst[3] = (float)nHeight;
	vConst[4] = fU1 * 0.5f + fU0;
	vConst[5] = fV1 * 0.5f + fV0;
	vConst[6] = fU1;
	vConst[7] = fV1;

	g_pDevice->m_pd3dDevice->SetVertexShaderConstantF( 0, vConst, 2 );
	g_pDevice->m_pd3dDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2,
											  g_QuadVerts, 3 * sizeof (float) );
}
