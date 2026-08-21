#include "Draw.h"

#include <string.h>

HRESULT CDraw::CreateOffscreen( LPDIRECTDRAWSURFACE7* ppSurface,
								int nWidth, int nHeight, BOOL bVideoMemory )
{
	LPDIRECTDRAWSURFACE7	pSurface;

	memset( &m_ddsd, 0, sizeof (DDSURFACEDESC2) );
	m_ddsd.dwSize			= sizeof (DDSURFACEDESC2);
	m_ddsd.dwFlags			= DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	m_ddsd.dwHeight			= nHeight;
	m_ddsd.dwWidth			= nWidth;
	m_ddsd.ddsCaps.dwCaps	= bVideoMemory
							? DDSCAPS_OFFSCREENPLAIN
							: ( DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY );

	m_hr = m_pDD->CreateSurface( &m_ddsd, &pSurface, NULL );

	if( m_hr )
		MessageBox( m_hWnd, "OffScreen 생성 실패", NULL, 0 );
	else
		*ppSurface = pSurface;

	return m_hr;
}

U16 CDraw::MakeColor( U8 r, U8 g, U8 b )
{
	if( m_b565 )
		return (U16)( (b >> 3) | ((g & 0xFC) << 3) | ((r & 0xF8) << 8) );

	return (U16)( (b >> 3) | ((g & 0xF8) << 2) | ((r & 0xF8) << 7) );
}

HRESULT CDraw::FillColor( LPDIRECTDRAWSURFACE7 pSurface, U16 wColor )
{
	DDBLTFX	bltfx;

	memset( &bltfx, 0, sizeof (bltfx) );
	bltfx.dwSize      = sizeof (DDBLTFX);
	bltfx.dwFillColor = wColor;

	m_hr = pSurface->Blt( NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx );
	if( !m_hr )
		return 0;

	MessageBox( m_hWnd, "FillColor fail", NULL, 0 );
	return m_hr;
}

HRESULT CDraw::SetColorKey( LPDIRECTDRAWSURFACE7 pSurface, U16 wColor )
{
	DDCOLORKEY	ck;

	ck.dwColorSpaceLowValue  = wColor;
	ck.dwColorSpaceHighValue = wColor;

	m_hr = pSurface->SetColorKey( DDCKEY_SRCBLT, &ck );
	if( !m_hr )
		return 0;

	MessageBox( m_hWnd, "ColorKeySetting fail", NULL, 0 );
	return m_hr;
}

BOOL CDraw::GetSurfacePointer( LPDIRECTDRAWSURFACE7 pSurface, void** ppBits, int* pPitch )
{
	memset( &m_ddsd, 0, sizeof (DDSURFACEDESC2) );
	m_ddsd.dwSize = sizeof (DDSURFACEDESC2);

	pSurface->Lock( NULL, &m_ddsd, DDLOCK_WAIT, NULL );

	*ppBits = m_ddsd.lpSurface;
	*pPitch = m_ddsd.lPitch >> 1;

	pSurface->Unlock( (LPRECT)m_ddsd.lpSurface );

	return ( *ppBits != NULL && *pPitch != 0 );
}

HRESULT CDraw::Blt( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
					LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh,
					BOOL bColorKey )
{
	RECT	rcSrc, rcDst;

	rcDst.left   = dx;			rcDst.top    = dy;
	rcDst.right  = dx + dw;		rcDst.bottom = dy + dh;

	rcSrc.left   = sx;			rcSrc.top    = sy;
	rcSrc.right  = sx + sw;		rcSrc.bottom = sy + sh;

	if( bColorKey )
		m_hr = pDst->Blt( &rcDst, pSrc, &rcSrc, DDBLT_WAIT | DDBLT_KEYSRC, NULL );
	else
		m_hr = pDst->Blt( &rcDst, pSrc, &rcSrc, DDBLT_WAIT, NULL );

	if( m_hr && m_hr == DDERR_SURFACELOST )
		m_hr = pDst->Restore();

	return m_hr;
}

HRESULT CDraw::BltMirror( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
						  LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh,
						  BOOL bColorKey )
{
	RECT	rcSrc, rcDst;
	DDBLTFX	bltfx;

	rcDst.left   = dx;			rcDst.top    = dy;
	rcDst.right  = dx + dw;		rcDst.bottom = dy + dh;

	rcSrc.left   = sx;			rcSrc.top    = sy;
	rcSrc.right  = sx + sw;		rcSrc.bottom = sy + sh;

	bltfx.dwSize = sizeof (DDBLTFX);
	bltfx.dwDDFX = DDBLTFX_MIRRORLEFTRIGHT;

	if( bColorKey )
		m_hr = pDst->Blt( &rcDst, pSrc, &rcSrc, DDBLT_WAIT | DDBLT_DDFX | DDBLT_KEYSRC, &bltfx );
	else
		m_hr = pDst->Blt( &rcDst, pSrc, &rcSrc, DDBLT_WAIT | DDBLT_DDFX, &bltfx );

	if( m_hr && m_hr == DDERR_SURFACELOST )
		m_hr = pDst->Restore();

	return m_hr;
}

U16 CDraw__AddPixel( CDraw* pDraw, U16 s, U16 d )
{
	U16	b = (U16)pDraw->m_AddTable5[   s & 0x1F         ][   d & 0x1F         ];
	U16	g = (U16)pDraw->m_AddTable6[ ( s >> 5 ) & 0x3F  ][ ( d >> 5 ) & 0x3F  ];
	U16	r = (U16)pDraw->m_AddTable5[   s >> 11          ][   d >> 11          ];

	return (U16)( b | ( 32 * ( g | ( r << 6 ) ) ) );
}

int CDraw::BltAdd( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
				   LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh )
{
	U16*	pDstBits;
	U16*	pSrcBits;
	int		nDstPitch, nSrcPitch;
	int		x, y;

	if( !pSrc || !pDst )
		return 0;

	if( dx <= 0 )
	{
		dw = dx + dw;
		sx = sw - dw;
		sw = dw;
		dx = 0;
	}
	if( dy <= 0 )
	{
		dh = dy + dh;
		sy = sh - dh;
		sh = dh;
		dy = 0;
	}

	if( dx >= DDRAW_SCREEN_W || dx + dw <= 0 ||
		dy >= DDRAW_SCREEN_H || dy + dh <= 0 )
		return 0;

	if( !GetSurfacePointer( pDst, (void**)&pDstBits, &nDstPitch ) )	return 0;
	if( !GetSurfacePointer( pSrc, (void**)&pSrcBits, &nSrcPitch ) )	return 0;

	for( y = 0; y < sh; y++ )
	{
		if( y + dy >= DDRAW_SCREEN_H )
			break;

		for( x = 0; x < sw; x++ )
		{
			U16		s;
			U16*	pd;

			if( x + dx >= DDRAW_SCREEN_W )
				break;

			s  = pSrcBits[ sx + ( sy + y ) * nSrcPitch + x ];
			pd = &pDstBits[ dx + ( dy + y ) * nDstPitch + x ];

			if( s != COLOR_KEY_565 )
				*pd = CDraw__AddPixel( this, s, *pd );
		}
	}

	return y;
}

int CDraw::BltAddMirror( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
						 LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh )
{
	U16*	pDstBits;
	U16*	pSrcBits;
	int		nDstPitch, nSrcPitch;
	int		x, y;

	if( dx <= 0 )
	{
		dw = dx + dw;
		sx = sw - dw;
		sw = dw;
		dx = 0;
	}
	if( dy <= 0 )
	{
		dh = dy + dh;
		sy = sh - dh;
		sh = dh;
		dy = 0;
	}

	if( dx >= DDRAW_SCREEN_W || dx + dw <= 0 ||
		dy >= DDRAW_SCREEN_H || dy + dh <= 0 )
		return 0;

	if( !GetSurfacePointer( pDst, (void**)&pDstBits, &nDstPitch ) )	return 0;
	if( !GetSurfacePointer( pSrc, (void**)&pSrcBits, &nSrcPitch ) )	return 0;

	for( y = 0; y < sh; y++ )
	{
		if( y + dy >= DDRAW_SCREEN_H )
			break;

		for( x = 0; x < sw; x++ )
		{
			U16		s;
			U16*	pd;

			if( x + dx >= DDRAW_SCREEN_W )
				break;

			s  = pSrcBits[ sx + ( sy + y ) * nSrcPitch + ( sw - x ) ];
			pd = &pDstBits[ dx + ( dy + y ) * nDstPitch + x ];

			if( s != COLOR_KEY_565 )
				*pd = CDraw__AddPixel( this, s, *pd );
		}
	}

	return y;
}

U16 CDraw__AlphaPixel( U16 s, U16 d, int nAlpha )
{
	int	nInv = 32 - nAlpha;
	int	rb   = ( ( ( nAlpha * ( s & 0xF81F ) ) >> 5 ) & 0xF81F )
			 + ( ( ( nInv   * ( d & 0xF81F ) ) >> 5 ) & 0xF81F );
	int	g    = ( nAlpha * ( ( s >> 5 ) & 0x3F ) + nInv * ( ( d >> 5 ) & 0x3F ) ) & 0x7E0;

	return (U16)( rb | g );
}

int CDraw::BltAlpha( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
					 LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh,
					 int nAlpha )
{
	U16*	pDstBits;
	U16*	pSrcBits;
	int		nDstPitch, nSrcPitch;
	int		x, y;

	if( dx >= DDRAW_SCREEN_W || dx + dw <= 0 ||
		dy >= DDRAW_SCREEN_H || dy + dh <= 0 )
		return 0;

	if( dx < 0 )			{ sx = -dx;  dw = dw + dx;  dx = 0; }
	if( dx + dw > m_nWidth )	dw = m_nWidth - dx;
	if( dy < 0 )			{ sy = -dy;  dh = dh + dy;  dy = 0; }
	if( dy + dh > m_nHeight )	dh = m_nHeight - dy;

	if( !GetSurfacePointer( pDst, (void**)&pDstBits, &nDstPitch ) )	return 0;
	if( !GetSurfacePointer( pSrc, (void**)&pSrcBits, &nSrcPitch ) )	return 0;

	for( y = 0; y < dh; y++ )
	{
		for( x = 0; x < dw; x++ )
		{
			U16		s  =  pSrcBits[ sx + ( sy + y ) * nSrcPitch + x ];
			U16*	pd = &pDstBits[ dx + ( dy + y ) * nDstPitch + x ];

			if( s == COLOR_KEY_565 )
				continue;

			*pd = CDraw__AlphaPixel( s, *pd, nAlpha );
		}
	}

	return y;
}

int CDraw::BltAlphaMirror( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
						   LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh,
						   int nAlpha )
{
	U16*	pDstBits;
	U16*	pSrcBits;
	int		nDstPitch, nSrcPitch;
	int		x, y;

	if( dx >= DDRAW_SCREEN_W || dx + dw <= 0 ||
		dy >= DDRAW_SCREEN_H || dy + dh <= 0 )
		return 0;

	if( dx < 0 )			{ sx = -dx;  dw = dw + dx;  dx = 0; }
	if( dx + dw > m_nWidth )	dw = m_nWidth - dx;
	if( dy < 0 )			{ sy = -dy;  dh = dh + dy;  dy = 0; }
	if( dy + dh > m_nHeight )	dh = m_nHeight - dy;

	if( !GetSurfacePointer( pDst, (void**)&pDstBits, &nDstPitch ) )	return 0;
	if( !GetSurfacePointer( pSrc, (void**)&pSrcBits, &nSrcPitch ) )	return 0;

	for( y = 0; y < dh - 1; y++ )
	{
		for( x = 0; x < dw; x++ )
		{
			U16		d  =  pDstBits[ dx + ( dy + y ) * nDstPitch + ( dw - x ) - 1 ];
			U16*	ps = &pSrcBits[ sx + ( sy + y ) * nSrcPitch + x ];

			if( d == COLOR_KEY_565 )
				continue;

			*ps = CDraw__AlphaPixel( d, *ps, nAlpha );
		}
	}

	return y;
}

BOOL CDevice::CheckFormat( D3DFORMAT Format )
{
	D3DDISPLAYMODE	Mode;

	m_pD3D->GetAdapterDisplayMode( 0, &Mode );

	return SUCCEEDED( m_pD3D->CheckDeviceFormat( 0, D3DDEVTYPE_HAL, Mode.Format,
												 0, D3DRTYPE_TEXTURE, Format ) );
}

U32 CDevice::SetBlend( U32 dwBlend )
{
	U32	dwOld = m_dwBlend;

	if( dwBlend == dwOld )
		return dwBlend;

	m_dwBlend = dwBlend;

	if( ( ( ( dwBlend >> 8 ) ^ ( dwOld >> 8 ) ) & 7 ) == 0 )
		return dwBlend;

	switch( dwBlend & 0x700 )
	{
	case BLEND_INV_SRC:
		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
		return m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR );

	case BLEND_ADD:
		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCCOLOR );
		return m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	case BLEND_MODULATE:
		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
		return m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );

	case BLEND_ADD_ALPHA:
		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		return m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

	case BLEND_ALPHA:
		m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
		return m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	}

	return dwBlend;
}

HRESULT CDraw::PutImage1555( void* pImage, int nLeft, int nTop, int nRight, int nBottom,
							 int nPitch, int nSrcHeight, LPDIRECTDRAWSURFACE7 pSurface )
{
	U16*	pSrc;
	U16*	pDst;
	int		nDstPitch;
	int		x, y;

	pSurface->Lock( NULL, &m_ddsd, DDLOCK_WAIT, NULL );

	nDstPitch = m_ddsd.lPitch >> 1;
	pDst      = (U16*)m_ddsd.lpSurface;

	pSrc = (U16*)pImage + nPitch * ( nSrcHeight - nTop - nBottom ) + 2 * nLeft;

	for( y = nTop; y < nBottom; y++ )
	{
		for( x = nLeft; x < nRight; x++ )
		{
			U16	s = pSrc[ x + nPitch * y ];

			if( s & 0x8000 )
				pDst[ y * nDstPitch + x ] = (U16)( ( s & 0x1F )
												 | ( ( s & 0xFC00 ) << 1 )
												 | ( ( ( s >> 5 ) & 0x1F ) << 6 ) );
		}
	}

	return pSurface->Unlock( (LPRECT)m_ddsd.lpSurface );
}

HRESULT CDraw::PutImage8888( void* pImage, int nX, int nY, int nWidth, int nHeight,
							 int nPitch, int nSrcHeight, LPDIRECTDRAWSURFACE7 pSurface )
{
	U8*		pSrc;
	U16*	pDst;
	int		nDstPitch;
	int		x, y;

	pSurface->Lock( NULL, &m_ddsd, DDLOCK_WAIT, NULL );

	nDstPitch = m_ddsd.lPitch >> 1;
	pDst      = (U16*)m_ddsd.lpSurface;
	pSrc      = (U8*)pImage;

	for( y = nY; y < nY + nHeight; y++ )
	{
		for( x = nX; x < nX + nWidth; x++ )
		{
			int	o = 4 * ( nPitch * y + x );
			U8	b = pSrc[o + 0];
			U8	g = pSrc[o + 1];
			U8	r = pSrc[o + 2];
			U8	a = pSrc[o + 3];

			if( a )
				pDst[ y * nDstPitch + x ] = (U16)( ( b >> 3 )
												 | ( ( g & 0xFC ) << 3 )
												 | ( ( r & 0xF8 ) << 8 ) );
		}
	}

	return pSurface->Unlock( (LPRECT)m_ddsd.lpSurface );
}
