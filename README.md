# Ghost Online asset reader — restored client source

The original loading code for Ghost Online `.spr` (sprite) and `.mot`
(animation) files, recovered from the retail client `Game.exe` and rewritten as
period-appropriate C++ — no STL, no namespaces, no templates, include guards,
the tab-and-column layout the 2002–2004 tool chain produced.

This is **the client's own reader**, not a reimplementation. Class and member
names are not invented: the retail build still ships its assert text, so the
reader names its own fields.

```
"\tif( !File.Read(&m_ColorType, sizeof (int)) )실패 "
"if( !File.Read(&m_TexBlendType, sizeof (int)) ) 실패 "
"\tif( !File.Read(&m_SearchColor, sizeof(RGBVal)) ) 실패 "
"if( !File.Read(&m_Width, sizeof (int)) ) 실패 "
"if( !File.Read( pImage, m_NumPixels * sizeof(U16)) ) 실패 "
"g_pDraw->CreateOffscreen( &m_pOffScreen, m_Width, m_Height, FALSE );"
```

That fixes `CFile File` as the parameter name, `File.Read` as the call, and the
seven image-header members with their exact spelling and order.

---

## Contents

| File | Classes | `sizeof` | Recovered from |
|---|---|---|---|
| `Define.h` | types, enums, globals | — | — |
| `File.h/.cpp` | `CFile` | 268 | `6CF690` `6CF6B0` `6CF6D0` `6CF820` `6CF840` `6CF880` `6CF8C0` |
| `Name.h/.cpp` | `CName` | 168 | `783C00` `783A20` `783A30` `783A40` `783A70` `783B80` |
| `Draw.h/.cpp` | `CDraw`, `CDevice` | partial | `7E3500` `7E47B0` `7E3640` `7E39E0` `7E37B0` `7E38C0` `7E4800` `7E3EF0` `7E6D00` `7E5B20` `7E5D60` `7E4CA0` `7E4F80` `6CAD40` `6CB280` |
| `Image.h/.cpp` | `CImage` | 236 | `6C81F0` `6C8380` `6C8410` `6C9250` `6C86D0` `6C8AD0` `6C9270` `6C84E0` `6C8150` `6C9520` `6C9AB0` `6C8FF0` |
| `Sprite.h/.cpp` | `CBox` `CFrame` `CSprite` | 16 / 24 / 516 | `506B80` `782290` `7834B0` `782EA0` `7831D0` `783320` `7835D0` |
| `Motion.h/.cpp` | `CMotFrame` `CAction` `CMotion` | 30 / 180 / 348 | `6DA9D0` `6DAA40` `6DAAD0` `6DAC60` `6DACD0` `6DADF0` `6DAF00` `6DAF70` `6DAFA0` `6DB120` `6DB270` |
| `MotionSet.h/.cpp` | `CMotionSet` | — | **not from the binary**, see [.cmo](#cmo) |
| `SpriteSet.h/.cpp` | `CSpriteSet` | — | layout from `4182E0` `41FA30`, wrapper is not the client's |

The `sizeof` column is not decoration — each one was confirmed against the
allocation the client makes (`operator new(236)` for a `CImage`, a 180-byte
`eh vector constructor iterator` stride for `CAction`, and so on), which is what
pins the member layout.

---

## Build

There is no project file. This is a source drop meant to be added to a host
project, eight `.cpp` files:

```
File.cpp  Name.cpp  Image.cpp  Sprite.cpp  Motion.cpp  Draw.cpp
MotionSet.cpp  SpriteSet.cpp
```

**Requirements**

| | |
|---|---|
| Compiler | MSVC. Verified on v145 (VS 2022); the code is plain enough for VC6. |
| Platform | **Win32 (x86)** — the classes model a 32-bit object layout |
| Character set | **MultiByte**, not Unicode. `MessageBox` must resolve to `MessageBoxA`. |
| `ddraw.h`, `d3d9.h` | Windows SDK |
| `d3dx9.h`, `d3dx9.lib` | legacy DirectX SDK, or the `Microsoft.DXSDK.D3DX` NuGet package — no longer in the Windows SDK |
| Libraries | `user32.lib` (for `MessageBoxA`), `d3dx9.lib` |

**Encoding.** Sources are UTF-8 with BOM and the Korean message text is inline.
Build with **`/execution-charset:.949`** so the narrow literals come out as the
CP949 bytes the retail image carries:

```bat
cl /c /W3 /EHsc /D_CRT_SECURE_NO_WARNINGS /execution-charset:.949 ^
   File.cpp Name.cpp Image.cpp Sprite.cpp Motion.cpp Draw.cpp ^
   MotionSet.cpp SpriteSet.cpp
```

Without that switch MSVC raises C4566 and substitutes `?` for every Korean
character on a non-Korean system codepage, and the message text stops matching
the original.

### It compiles; it does not link on its own

By design. The reader and the draw path are one half of the client — they call
into the renderer's own state, the loader-thread bookkeeping and the D3DX helper
set, and those are the host's job. Linking the eight objects alone leaves
**28 unresolved externals**, and that list *is* the integration contract.

Satisfying it is not much work: a **60-line stub host** — the globals below plus
twelve empty functions — links the library and runs it. That is how the readers
here were checked against real assets, not just compiled.

**Globals to define (13)**

```cpp
CDraw*   g_pDraw;           // DirectDraw7 back end
CDevice* g_pDevice;         // Direct3D9 back end
BOOL     g_bDDraw;          // TRUE selects g_pDraw, FALSE selects g_pDevice
int      g_nTextureCount;   // live texture count, for leak tracking only
int      xRight, yBottom;   // screen size; seeds CImage::m_ClipRect
RECT     g_rcClip;          // scissor the D3D draw path tests against
float    g_QuadVerts[4][3]; // unit quad, TRIANGLEFAN; see Drawing below
CLock    g_LoadLock, g_MapLoadLock;
int      g_nLoadThread, g_nMapLoadThread, g_nLoadedSprite;
```

Setting the three loader-thread ints to `0` disables the mutex path entirely and
is the right choice for a single-threaded tool.

**Functions to implement (12)**

| Symbol | Needed for |
|---|---|
| `CDraw::PutImage565` | ColorType 1 upload — the one uploader still missing |
| `CLock::Lock` `Unlock` | only reached when a loader-thread flag is non-zero |
| `PutImage16` | the A8R8G8B8 → A4R4G4B4 fallback |
| `GetOption` | index 2 picks keyed vs alpha blitting, index 3 gates the box warning |
| `MatrixIdentity` `MatrixRotationX/Y/Z` `MatrixTranslate` `MatrixTranspose` `MatrixMultiply` | the rotated draw path; plain `D3DX*` equivalents |

**From libraries (3)** — `MessageBoxA` (`user32.lib`), `D3DXCreateTexture` and
`D3DXCreateTextureFromFileInMemoryEx` (`d3dx9.lib`).

Everything else `CDraw` and `CDevice` need is now in `Draw.cpp`. If all you want
is to *parse* files — no surfaces, no textures, no drawing — pass
`bCreateSurface = FALSE` to `CSprite::Read` and stub the twelve with empty
bodies. Nothing in the parsing path calls them.

---

## Use

### Read a sprite

```cpp
CFile   File;
CSprite Sprite;

if( File.Open( "data\\Avatar\\weapon_m_h.spr", FILE_READ ) )
{
    Sprite.Read( File, TRUE );      // FALSE = parse only, no surfaces
    File.Close();
}

for( int i = 0; i < Sprite.m_nFrameNum; i++ )
{
    CFrame& Frame = Sprite.m_pFrame[i];
    if( !Frame.m_pImage )           // empty placeholder frame
        continue;

    CImage* pImage = Frame.m_pImage;
    // pImage->m_Width, m_Height, m_ColorType, m_TexBlendType,
    // pImage->m_pOffScreen  or  pImage->m_pTexture
}
```

`CSprite::Read` takes a `CFile` that is already open. It frees whatever it held
before, so the same object can be reused. `bCreateSurface` decides whether the
pixel payload is decoded into a surface or seeked past — parsing headers is
cheap either way.

`CImage::LoadFromFile` exists for a single image in a file of its own; it is not
how `.spr` files are read.

### Read an animation

```cpp
CMotion Motion;
Motion.LoadFromFile( "data\\Avatar\\weapon_m_h.mot" );

for( int a = 0; a < Motion.m_ActionNum; a++ )
{
    CAction& Action = Motion.m_pAction[a];
    // Action.m_Name.m_Name is CP949

    for( int f = 0; f < Action.m_FrameNum; f++ )
    {
        CMotFrame& Rec = Action.m_pFrame[f];
        // Rec.m_ImageNum indexes Sprite.m_pFrame[] — an ordinal, see below
        // Rec.m_Delay is in 40 Hz ticks (25 ms)
        // Rec.m_X, Rec.m_Y are the anchor
    }
}
```

`CMotion::LoadFromFile` opens the file itself. The two are joined only by base
name: `foo.spr` and `foo.mot`.

### Read a .csp

```cpp
CSpriteSet Set;

if( Set.Open( "data\\Avatar\\hat.csp" ) )
{
    for( int i = 0; i < Set.m_nSpriteNum; i++ )
    {
        CSprite Sprite;
        Set.ReadSprite( i, &Sprite, FALSE );   // TRUE to build surfaces

        int nID[16];
        int n = Set.ReadID( i, nID, 16 );      // 0 when the pack has no ids
    }
}
```

`Open` reads only the count and the seek table and leaves the file open;
`ReadSprite` seeks to one entry and hands it to `CSprite::Read`, the same call a
standalone `.spr` goes through. Reading entry 900 costs one seek, not a walk
through the previous 899.

### Read a .cmo

```cpp
CMotionSet Set;

if( Set.LoadFromFile( "data\\OBJ\\PET\\ydoll_041_1.cmo" ) )
{
    for( int i = 0; i < Set.m_nMotionNum; i++ )
    {
        CMotion* pMotion = Set.GetMotion( i );
        int      nID     = Set.GetID( i );   // -1 when the file carries no ids
    }
}
```

`LoadFromFile` tries the plain layout, then the id-prefixed one, and accepts
whichever lands exactly on the end of the file. It returns `FALSE` rather than a
partial set — see [.cmo](#cmo) for why five shipped files fail.

### Draw a frame

```cpp
CImage* pImage = Sprite.m_pFrame[ Rec.m_ImageNum ].m_pImage;

pImage->Draw( x + Rec.m_X, y + Rec.m_Y,
              bFacingLeft ? -1 : 1,      // nFlip
              1.0f,                      // scale
              0.0f,                      // angle, degrees
              0xFFFFFFFF,                // top 5 bits are the alpha level
              0.0f, 0.0f );              // rotation centre
```

Which back end runs is decided by `g_bDDraw`, not by the caller.

### Placement

The engine draws a frame at

```
position = origin + anchor - size / 2
```

Anything that previews these assets has to use the same rule or it will disagree
with the game.

### Ownership

Every class owns what it allocates and frees it in its destructor, so a
`CSprite` going out of scope releases its frames, their images and their
DirectDraw surfaces or D3D textures. There are no copy constructors — copy a
`CSprite` and you get a double free. That is how the original stood.

---

## How it works

**`CFile`** wraps the CRT low-level calls (`_open` / `_read` / `_write` /
`_lseeki64`). Its `m_FileName` is kept so the loader can name a file in a
message box when a frame fails to parse.

**`CName`** is the base for every named object: a 160-byte buffer plus 8 bytes
the loader never touches. `SetName` memsets all 160 before it copies, which is
what pins the size.

**`CImage`** is the sprite image reader and the interesting one. `Read`
dispatches on `g_bDDraw`:

- `ReadDDraw` (`0x6C86D0`) — creates a DirectDraw7 offscreen surface, fills it
  magenta, sets a magenta colour key, then hands the pixels to one of the three
  `CDraw::PutImage*` uploaders. Every failure raises a message box quoting the
  source line that failed, which is why the member names survived.
- `ReadD3D` (`0x6C8AD0`) — creates a Direct3D 9 **managed** texture and copies
  rows straight into a `LockRect`, because the file's pixel layouts already
  *are* D3D formats. DXT payloads go in through
  `D3DXCreateTextureFromFileInMemoryEx`. If the card cannot do A8R8G8B8 it drops
  to A4R4G4B4 by hand.

It is Direct3D **9**, not 8 — the texture vtable offsets 68/76/80 are
`GetLevelDesc` / `LockRect` / `UnlockRect`, which sit 12 bytes lower in D3D8.

**`CSprite` → `CFrame` → `CImage`** is the containment chain, and
`CMotion` → `CAction` → `CMotFrame` is the animation side. Neither references
the other; `CMotFrame::m_ImageNum` is a bare index the caller resolves.

**`Draw.h/.cpp`** carry `CDraw` and `CDevice` as far as the read and draw paths
reach. Both are far larger in the retail image; the offsets in the comments are
the real ones so the rest can be filled in later without moving anything. The
member layout is padded to put every named field at its real offset —
`m_pBack` +60, `m_ddsd` +2072, `m_b565` +2220, `m_nWidth`/`m_nHeight`
+2224/+2228, `m_hr` +2276, and the two saturating add tables at +30964 and
+84212.

### Drawing

`CImage::Draw` has two overloads, one taking separate X and Y scales
(`0x6C9520`) and one taking a single scale (`0x6C9AB0`). Both branch on
`g_bDDraw`.

**DirectDraw.** The direction comes from the `nFlip` argument when the angle is
zero, and from the sign of the angle otherwise — anything outside
`(-270, -90)` draws unmirrored. That direction crossed with `m_TexBlendType`
and `GetOption(2)` picks one of six blits:

| | straight | mirrored |
|---|---|---|
| additive (`m_TexBlendType == 1`) | `BltAdd` `7E5B20` | `BltAddMirror` `7E5D60` |
| keyed (`GetOption(2)` false) | `Blt` `7E3EF0` | `BltMirror` `7E6D00` |
| alpha | `BltAlpha` `7E4CA0` | `BltAlphaMirror` `7E4F80` |

The first two rows go through `IDirectDrawSurface7::Blt` — `DDBLT_KEYSRC` for
the keyed case, `DDBLTFX_MIRRORLEFTRIGHT` for the mirrored one — and call
`Restore()` on `DDERR_SURFACELOST`. The other four are hand-written 5:6:5 pixel
loops that key on `0xF81F`, clip to a hardcoded 800×600, and reach the surface
through `GetSurfacePointer` (`7E4800`), which locks, takes the bits pointer and
pitch, and unlocks again before returning them. Additive uses the two lookup
tables the object carries; alpha carries red and blue together under a
`0x07E0F81F` mask and does green separately, with the level in the top 5 bits of
`dwColor`.

**Direct3D 9.** Nothing builds a quad per draw. `g_QuadVerts` is a unit quad
centred on the origin, drawn as a `TRIANGLEFAN` with FVF `D3DFVF_XYZ` and a
stride of 12; the placement, size and UVs go in as two vertex shader constants
and the shader expands them. `DrawPrimitive` (`0x6C8FF0`) centres on
`0.5 * w + x`, which is the same rule the animation side states as
`origin + anchor - size/2`. `m_TexBlendType` selects the blend through
`CDevice::SetBlend` (`0x6CB280`), which caches the last value and only touches
`D3DRS_SRCBLEND` / `D3DRS_DESTBLEND` when bits 8..10 change. A rotated draw
builds a world matrix, uploads it to constants `c4..c7` and restores identity
afterwards; a centre component of 10000 or more is a flag selecting the X or Y
rotation axis instead of Z.

Horizontal flip on this path is not a matrix — it negates the width and offsets
by `m_DrawWidth * fScale`.

---

## File formats

Measured, not guessed — from `Game.exe` in IDA and from scanning the shipped
corpus. Text is **CP949 (EUC-KR)** everywhere, including paths, regardless of UI
locale. All integers are little-endian.

### `.spr`

```
0x000  char  path[128]      original asset path, NUL terminated (+ leftover bytes)
0x080  char  desc[128]      description / nickname              (+ leftover bytes)
0x100  int32 frameCount
0x104  frames, back to back — there is NO offset table
       ... optional trailing bytes after the last frame
```

Per frame:

| Field | Type | Meaning |
|---|---|---|
| `frameIndex` | int32 | the frame's own ordinal. Matches its position in 89% of frames — and is **not** what animation records point at. |
| `boxCount` | int32 | hit boxes that follow |
| `boxes` | int32 × 4 | `left, top, right, bottom`, anchor-relative. Zero area = an attach point. Refused above 10. |
| `atkBoxCount` | int32 | |
| `atkBoxes` | int32 × 4 | second list, same rule |
| `hasImage` | uint8 | **0 ⇒ the frame ends here.** No image header follows. Reading one anyway desyncs every later frame. |
| `m_ColorType` | int32 | see below |
| `m_TexBlendType` | int32 | 0 = normal, 1 = additive |
| `m_FileType` | int32 | `D3DXIMAGE_FILEFORMAT` of the source art: 0 BMP, 1 JPG, 2 TGA, 3 PNG, 4 DDS |
| `m_SearchColor` | uint8 × 3 | magenta (255, 0, 255) in every frame scanned |
| `m_Width` | uint16 | |
| *(spill)* | uint8 × 2 | not a field, see below |
| `m_Height` | uint16 | |
| *(spill)* | uint8 × 2 | not a field, see below |
| `m_NumPixels` | int32 | |
| pixels | raw | `m_NumPixels × 2` or `× 4` by ColorType |

**The image header is 27 bytes, and the four "unknown" bytes are not a field.**
`m_Width` and `m_Height` are `U16` members read with `sizeof (int)`:

```cpp
File.Read( &m_Width,  sizeof (int) );   // 4 bytes into a U16
File.Read( &m_Height, sizeof (int) );   // 4 bytes into a U16
```

The high half of the width read lands in `m_Height`; the high half of the height
read lands in `m_DrawWidth`. Both are overwritten before the function returns,
so the bug never shows at runtime — but it is why the header is 27 bytes and not
28, and it identifies the two 2-byte gaps as adjacent-member spill rather than
data. Any writer must preserve them verbatim.

**Box field order.** On disk a box is `left, top, right, bottom`. `CBox` is
`{ m_nLeft, m_nRight, m_nTop, m_nBottom }`, which is why `CFrame::Read` fills
offsets `+0, +8, +4, +12` in that order and looks shuffled. Two independent
confirmations: `CFrame::GetBox` (`0x7831D0`) pairs members 0 and 1 with the X
axis and 2 and 3 with Y; and of 4006 non-empty boxes across 1406 shipped
sprites, all 4006 satisfy `left ≤ right ∧ top ≤ bottom` under the disk order and
only 1577 under any other.

### ColorType

| Value | `Define.h` | Format | D3D format | Bytes/px |
|---|---|---|---|---|
| 1 | `COLOR_TYPE_RGB565` | RGB565 / RGB555 | `D3DFMT_A1R5G5B5` | 2 |
| 3 | `COLOR_TYPE_A1R5G5B5` | A1R5G5B5 | `D3DFMT_A1R5G5B5` | 2 |
| 4 | `COLOR_TYPE_A8R8G8B8` | A8R8G8B8, source order B G R A | `D3DFMT_A8R8G8B8` | 4 |
| 5 | `COLOR_TYPE_A4R4G4B4` | A4R4G4B4 | `D3DFMT_A4R4G4B4` | 2 |
| 6–10 | `COLOR_TYPE_DXT1`…`DXT5` | DXT1–DXT5 | `D3DFMT_DXT1`…`DXT5` | — |

A DXT payload is a whole `.dds` preceded by its own int32 byte count, and only
that case has the extra count. Only 1, 3 and 4 are reachable on the DirectDraw
path; the D3D path handles all of them. Anything else raises
"픽셀포멧을 찾을수 없습니다."

**Transparency.** `CDraw::PutImage1555` (`0x7E37B0`) writes a pixel only when
**bit 15 is set** — alpha bit clear means skip. `0x8000` (alpha set, RGB 0) is
opaque black, which is what preserves outlines. For A8R8G8B8 the test is
`alpha != 0`.

`m_SearchColor` is read and then **never consumed** — the DirectDraw path keys
on a hardcoded `MakeColor(255, 0, 255)`. Magenta is an authoring convention, not
a per-file setting.

### `.mot`

Holds no pixels. Every record is an index into the `.spr` of the same base name.

```
0x000  char  path[128]
0x080  char  desc[128]
0x100  int32 m_ActionNum        <-- same offset as the sprite's frame count
       per action:
         int32 m_ID             zero in every shipped file
         char  szName[20]       CP949, run through CName::SetName
         int32 m_FrameNum
         m_FrameNum × 30-byte CMotFrame
```

No count and no magic sits ahead of the two name fields, which is why the action
count lands at exactly `0x100`. A `.mot` ends on its last record — every shipped
file parses to exact EOF with nothing left over.

`CMotFrame` is 15 × int16, read as one block. The constructor (`0x6DA9D0`)
defaults split it into `6 + 4 + 4 + 1`: each overlay group ends in a member that
defaults to its own slot number, 1 then 2. That is what slots 9 and 13 are.

| Off | Member | Default | In the corpus |
|---|---|---|---|
| 0 | `m_ImageNum`, −1 = none | 0 | ordinal into the paired `.spr` |
| 2 | `m_Delay`, 40 Hz ticks (25 ms) | 0 | 0–400, most often 4 or 8 |
| 4 | `m_X` anchor | 0 | signed |
| 6 | `m_Y` anchor | 0 | signed, mostly negative — sprites hang above the origin |
| 8 | `m_ImageNum2` | 0 | equals `m_ImageNum` in 99.8% of records |
| 10 | `m_Type` | 0 | only `{−1, 0, 6, 14, 15}`, non-zero only on avatar parts |
| 12 | `m_SubImage1` | **−1** | −1 in 99.98% |
| 14, 16 | `m_SubX1`, `m_SubY1` | 0 | |
| 18 | `m_SubType1` | **1** | 0 in every shipped record |
| 20 | `m_SubImage2` | **−1** | −1 in 98.5%; real indices on `ydoll_*` |
| 22, 24 | `m_SubX2`, `m_SubY2` | 0 | |
| 26 | `m_SubType2` | **2** | 0 in every shipped record |
| 28 | `m_Event`, 0 = none | 0 | sound / effect trigger, concentrated on `ATTACK_*` |

Action names seen: `STAND_1/2/3`, `WALK_1`, `RUN_1`, `ATTACK_1/2`, `DAMAGE_1/2`,
`DEAD_1/2`, `JUMP_1`, `LJUMP_1/2`, `START_1`, `AREADY_1`, `ASTAND_1`, `ANGRY_1`,
plus empty names for unused slots.

**`m_ImageNum` is an array ordinal, not a lookup on the sprite's own
`frameIndex`.** This matters because 198 shipped sprites carry frames whose
`frameIndex` is not `0..n-1`. Across those, 1288 references resolve as an
ordinal only and **zero** resolve as a `frameIndex` only. It is also not a
counter — `STAND_1` of `m006012` runs `0,1,2,3,4,3,2,1,0,0`. A further 82
references point past the end of their sprite and are simply dangling; the
client survives them because every accessor bounds-checks first.

### `.csp`

A pack of whole `.spr` files with a seek table in front, so one sprite can be
pulled out without touching the rest. It is to `.spr` what `.cmo` is to `.mot`,
except that this one has an offset table and `.cmo` does not.

```
0x000  int32    m_nSpriteNum
0x004  __int64  Offset[ m_nSpriteNum ]     absolute; Offset[0] == 4 + 8*n
       per entry, at Offset[i]:
         int32  (always 0)          \
         int32  nIDNum               >   present in some containers only
         int32  ID[ nIDNum ]        /
         CSprite                        the same record a .spr holds
```

Unlike `.cmo`, this layout **is** confirmed against `Game.exe`. Two loaders show
the seek arithmetic outright:

```
4182E0   lseeki64( Offset[i] + 4 );   lseeki64( 4 * nIDNum + 4 );   CSprite::Read
41FA30   lseeki64( Offset[i] );                                     CSprite::Read
```

— which is exactly "skip `8 + 4*nIDNum`, or skip nothing". The client never
reads the id list back; it already holds it in memory and uses the count only to
step over the copy in the file. What was *not* found is a container class: the
offset table lives in a preloaded global and entries are read one at a time on
demand, so the name `CSpriteSet` is mine even though the format is recovered.

Nothing in the file flags which of the two shapes it is, because the client
knows which pack it opened. A reader has to tell them apart, and the first byte
of an entry does it: a name never starts with NUL and the prefix always does,
since its first int is always 0. That rule accounts for **all 77** shipped
`.csp`, whose 9,276 entries and 123,628 frames every one end exactly where the
next offset begins.

`nIDNum` is 1 in the overwhelming majority of entries, but 0, 2, 5, 21 and 30
all occur — which is why the prefix is variable length and not the fixed 12
bytes it looks like at first.

### `.cmo`

**Derived from the files, not recovered from the binary.** Every other format
note here comes from `Game.exe`; this one does not. No caller of
`CMotion::Read` loops over entries, the `.cmo` path strings have no usable
cross references in either database, and no container function was found. The
class name `CMotionSet` is therefore invented.

What the corpus shows is simpler than the earlier guess — there is **no padding
at all**. Entries sit back to back and the file ends on the last one:

```
variant A   int32 m_nMotionNum
            CMotion  Motion[ m_nMotionNum ]

variant B   int32 m_nMotionNum
            per entry:  int32 (0)  int32 (1)  int32 nID  CMotion
```

| Variant | Files |
|---|---|
| A, plain | 44 |
| B, id-prefixed | 4 |
| not handled | 5 |

**48 of the 53** shipped `.cmo` parse to **exact EOF**. That replaces the older
reading of "variable inter-entry padding, rule unsolved" — the padding was an
artifact of probing for entry starts instead of trusting the count. In variant B
the prefix is always `{0, 1, nID}` and the id matches the entry's own name:
`attack_1001.mot` carries 1001.

The five that do not fit are `attack.cmo`, `attack_2D.cmo`, `normal.cmo`,
`normal_2D.cmo` (all under `data\OBJ\Effect`) and `itemicon.cmo`.
`CMotionSet::LoadFromFile` reports failure on them rather than guessing.

### Leftover bytes in the name fields

Every shipped `.spr` and `.mot` carries uninitialised process memory after the
NUL of its 128-byte name fields. The original tool kept the buffer on the stack
and wrote all 128 bytes without clearing it. Two rules were measured:

- **The description field ends with the tail of the path.** Both strings were
  copied into the *same* buffer and the shorter only overwrites the head.
  Verified on 791 of 798 files.
- **What follows is a real stack frame** — an ASCII remnant of the previous
  longer string, small counts, the literal `0x00000003`, then alternating stack
  and heap pointers at fixed deltas.

Anything that rewrites these files must reproduce the original bytes verbatim,
or accept that it is no longer producing something the original tool could have
produced.

---

## Faithful warts

Reproduced deliberately, each marked at its site:

- `COLOR_TYPE_RGB565` allocates `m_NumPixels * sizeof (U32)` and reads
  `m_NumPixels * sizeof (U16)` into it.
- `CImage::CreateTexture` leaks `pData` when the DXT read fails; the two
  `pImage` buffers in `ReadDDraw` leak on their failure paths.
- `CFile::Read` rejects `FILE_CLOSE` and `FILE_WRITE` but not `FILE_WRITE_TEXT`.
- **`CFile::Close` does not reset `m_Mode`.** So `Close()` followed by the
  destructor closes the same descriptor twice — and `CImage::LoadFromFile`,
  `CMotion::LoadFromFile` and `CSprite::LoadFromFile` all do exactly that. The
  CRT of the era shrugged; a modern one raises
  `STATUS_INVALID_CRUNTIME_PARAMETER` and kills the process. `CFile` is left
  alone; `CSpriteSet` and `CMotionSet` reset `m_Mode` by hand instead, in code
  that is mine rather than the client's. A host that calls the client's own
  `LoadFromFile` needs its own guard.
- `CName::operator=` is `sprintf( m_Name, rhs.m_Name )` — a name containing `%`
  is read as a conversion.
- `CAction::Copy` silently caps at 100 frames.
- `CMotion::AddMotion` calls `Release()`, which NULLs `m_pAction`, then only
  copies while `m_pAction` is non-NULL — so its loop body is unreachable on that
  path.
- `CFrame::Read` returns `FALSE` for a frame with no image. `CSprite::Read`
  discards the result, so it is not an error.

---

## Not restored

`CDraw::PutImage565` (`0x7EB4A0`), and the rest of the `CDraw` translation unit.

That unit holds roughly 240 functions between `0x7E0000` and `0x7F5000`, almost
all of them hand-tuned pixel loops. The ones the read and draw paths actually
reach are restored here; the rest are not, and no prototype is invented for
them — of a sampled slice only 1 function in 14 carries enough type information
to recover a signature without decompiling it, and 9 of 14 had no callers at
all. A table of plausible-looking prototypes would be the one thing this
library is built to avoid.

The `.cmo` container is implemented but its loader was never found, so
`CMotionSet` is a reader that matches the data rather than the client's code.
`.csp` is the better case: the format comes from `4182E0` and `41FA30`, and only
the wrapper is invented.

## Still unknown

- `CMotFrame::m_Type` (slot 5) — a small enum, non-zero only on avatar parts.
  The draw path does not read it, so it is consumed somewhere further up.
- Which of `m_ImageNum2`, `m_SubImage1`, `m_SubImage2` corresponds to which
  facing or layer at runtime. `CImage::Draw` takes the flip from its own
  argument and from the sign of the angle, never from the record, so the
  answer is in the caller rather than here.
- Why `frameIndex` disagrees with the frame's ordinal in 11% of frames, given
  nothing reads it as a key.
- The layout of the 5 `.cmo` files neither variant explains.
