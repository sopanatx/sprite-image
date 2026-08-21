#ifndef	__DEFINE_H__
#define	__DEFINE_H__

#include <windows.h>

typedef	unsigned char		U8;
typedef	unsigned short		U16;
typedef	unsigned long		U32;
typedef	signed char			S8;
typedef	signed short		S16;
typedef	signed long			S32;

struct	RGBVal
{
	U8		r;
	U8		g;
	U8		b;
};

enum
{
	COLOR_TYPE_RGB565		= 1,
	COLOR_TYPE_A1R5G5B5		= 3,
	COLOR_TYPE_A8R8G8B8		= 4,
	COLOR_TYPE_A4R4G4B4		= 5,
	COLOR_TYPE_DXT1			= 6,
	COLOR_TYPE_DXT2			= 7,
	COLOR_TYPE_DXT3			= 8,
	COLOR_TYPE_DXT4			= 9,
	COLOR_TYPE_DXT5			= 10
};

enum
{
	TEX_BLEND_NORMAL		= 0,
	TEX_BLEND_ADD			= 1
};

enum
{
	FILE_TYPE_BMP			= 0,
	FILE_TYPE_JPG			= 1,
	FILE_TYPE_TGA			= 2,
	FILE_TYPE_PNG			= 3,
	FILE_TYPE_DDS			= 4
};

enum
{
	FILE_READ				= 0,
	FILE_WRITE				= 1,
	FILE_READ_TEXT			= 2,
	FILE_WRITE_TEXT			= 3,
	FILE_CLOSE				= 5
};

class	CDraw;
class	CDevice;

extern	CDraw*		g_pDraw;
extern	CDevice*	g_pDevice;
extern	BOOL		g_bDDraw;
extern	int			g_nTextureCount;
extern	int			xRight;
extern	int			yBottom;

class	CLock
{
public:
	HANDLE	m_hOwner;
	HANDLE	m_hMutex;

	void	Lock();
	void	Unlock();
};

extern	CLock	g_LoadLock;
extern	CLock	g_MapLoadLock;
extern	int		g_nLoadThread;
extern	int		g_nMapLoadThread;
extern	int		g_nLoadedSprite;

extern	BOOL	GetOption( int nIndex );

BOOL	PutImage16(	void* pDst, int nSrcPitch, int nDstWidth, int nDstHeight,
					U16* pSrc, int nSrcWidth, int nSrcHeight,
					int nX, int nY, COLORREF crKey );

#endif
