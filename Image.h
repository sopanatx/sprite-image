#ifndef	__IMAGE_H__
#define	__IMAGE_H__

#include "Define.h"
#include "Name.h"
#include "File.h"
#include "Draw.h"

class	CImage : public CName
{
public:
	int						m_ColorType;
	int						m_TexBlendType;
	int						m_FileType;
	RGBVal					m_SearchColor;
	RECT					m_ClipRect;
	LPDIRECT3DTEXTURE9		m_pTexture;
	LPDIRECTDRAWSURFACE7	m_pOffScreen;
	U16						m_Width;
	U16						m_Height;
	U16						m_DrawWidth;
	U16						m_DrawHeight;
	int						m_StartX;
	int						m_StartY;
	int						m_NumPixels;
	int						m_Reserved2;
	int						m_Reserved3;

			CImage();
			~CImage();

	void	Release();

	BOOL	Read( CFile& File, BOOL bCreateSurface );

	BOOL	ReadDDraw( CFile& File, BOOL bCreateSurface );

	BOOL	ReadD3D( CFile& File, BOOL bCreateSurface );

	BOOL	LoadFromFile( const char* pFileName );

	BOOL	CreateTexture( LPDIRECT3DTEXTURE9* ppTexture, int nWidth, int nHeight,
						   D3DFORMAT Format, CFile& File, U32 nDataSize );

	static void	SkipImage( CFile& File, int nNumPixels, int nColorType, U32 nDataSize );

	void	Draw( int x, int y, char nFlip, float fScaleX, float fScaleY,
				  float fAngle, U32 dwColor, float fCenterX, float fCenterY );

	void	Draw( int x, int y, char nFlip, float fScale,
				  float fAngle, U32 dwColor, float fCenterX, float fCenterY );

	void	DrawPrimitive( int x, int y, int nWidth, int nHeight,
						   float fU0, float fV0, float fU1, float fV1,
						   U32 dwColor, int nStage );
};

#endif
