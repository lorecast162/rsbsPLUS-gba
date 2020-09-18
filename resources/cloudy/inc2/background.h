
//{{BLOCK(background)

//======================================================================
//
//	background, 256x256@8, 
//	+ palette 256 entries, not compressed
//	+ 1003 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 32x32 
//	Total size: 512 + 64192 + 2048 = 66752
//
//	Time-stamp: 2020-09-19, 01:12:09
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.15
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_BACKGROUND_H
#define GRIT_BACKGROUND_H

#define backgroundTilesLen 64192
extern const unsigned int backgroundTiles[16048];

#define backgroundMapLen 2048
extern const unsigned short backgroundMap[1024];

#define backgroundPalLen 512
extern const unsigned short backgroundPal[256];

#endif // GRIT_BACKGROUND_H

//}}BLOCK(background)
