#ifndef	__DRAW_H__
#define	__DRAW_H__

#include "Define.h"

#include <ddraw.h>
#include <d3d9.h>
#include <d3dx9.h>

#define	DDRAW_SCREEN_W		800
#define	DDRAW_SCREEN_H		600

#define	COLOR_KEY_565		0xF81F

enum
{
	BLEND_ALPHA			= 0x0000,
	BLEND_ADD_ALPHA		= 0x0100,
	BLEND_MODULATE		= 0x0200,
	BLEND_INV_SRC		= 0x0300,
	BLEND_ADD			= 0x0400
};

class	CDraw
{
public:
	char					m_Pad0000[52];
	LPDIRECTDRAW7			m_pDD;
	char					m_Pad0038[4];
	LPDIRECTDRAWSURFACE7	m_pBack;
	char					m_Pad003C[2008];
	DDSURFACEDESC2			m_ddsd;
	char					m_Pad0894[24];
	BOOL					m_b565;
	int						m_nWidth;
	int						m_nHeight;
	char					m_Pad08B8[36];
	HWND					m_hWnd;
	char					m_Pad08DC[4];
	HRESULT					m_hr;
	char					m_Pad08E4[28684];
	U32						m_AddTable5[32][32];
	char					m_Pad88F4[49152];
	U32						m_AddTable6[64][64];

	HRESULT	CreateOffscreen( LPDIRECTDRAWSURFACE7* ppSurface,
							 int nWidth, int nHeight, BOOL bVideoMemory );
	U16		MakeColor( U8 r, U8 g, U8 b );
	HRESULT	FillColor( LPDIRECTDRAWSURFACE7 pSurface, U16 wColor );
	HRESULT	SetColorKey( LPDIRECTDRAWSURFACE7 pSurface, U16 wColor );

	HRESULT	PutImage1555( void* pImage, int nLeft, int nTop, int nRight, int nBottom,
						  int nPitch, int nSrcHeight, LPDIRECTDRAWSURFACE7 pSurface );
	HRESULT	PutImage565 ( void* pImage, int nLeft, int nTop, int nRight, int nBottom,
						  int nPitch, int nSrcHeight, LPDIRECTDRAWSURFACE7 pSurface );
	HRESULT	PutImage8888( void* pImage, int nX, int nY, int nWidth, int nHeight,
						  int nPitch, int nSrcHeight, LPDIRECTDRAWSURFACE7 pSurface );

	BOOL	GetSurfacePointer( LPDIRECTDRAWSURFACE7 pSurface,
							   void** ppBits, int* pPitch );

	HRESULT	Blt      ( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
					   LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh,
					   BOOL bColorKey );
	HRESULT	BltMirror( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
					   LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh,
					   BOOL bColorKey );

	int		BltAdd         ( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
							 LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh );
	int		BltAddMirror   ( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
							 LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh );
	int		BltAlpha       ( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
							 LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh,
							 int nAlpha );
	int		BltAlphaMirror ( LPDIRECTDRAWSURFACE7 pSrc, int sx, int sy, int sw, int sh,
							 LPDIRECTDRAWSURFACE7 pDst, int dx, int dy, int dw, int dh,
							 int nAlpha );
};

class	CDevice
{
public:
	char					m_Pad0000[4];
	LPDIRECT3D9				m_pD3D;
	LPDIRECT3DDEVICE9		m_pd3dDevice;
	char					m_Pad000C[5504];
	U32						m_dwBlend;

	BOOL	CheckFormat( D3DFORMAT Format );
	U32		SetBlend( U32 dwBlend );
};

extern	RECT	g_rcClip;
extern	float	g_QuadVerts[4][3];

void	MatrixIdentity ( D3DXMATRIX* pOut );
void	MatrixRotationZ( D3DXMATRIX* pOut, float fRadian );
void	MatrixRotationX( D3DXMATRIX* pOut, float fRadian );
void	MatrixRotationY( D3DXMATRIX* pOut, float fRadian );
void	MatrixTranslate( D3DXMATRIX* pOut, float x, float y, float z );
void	MatrixTranspose( D3DXMATRIX* pOut, D3DXMATRIX* pIn );
void	MatrixMultiply ( D3DXMATRIX* pOut, D3DXMATRIX* pA, D3DXMATRIX* pB );

#endif
