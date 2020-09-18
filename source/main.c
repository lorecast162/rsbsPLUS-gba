//SPDX-License-Identifier: BSD-3-Clause
//SPDX-FileCopyrightText: 2020 Lorenzo Cauli (lorecast162)

//enable multiboot?
#define MULTIBOOT int __gba_multiboot;
MULTIBOOT

typedef unsigned short u16;

//include spheres data and palette
#include "sphere.h"
#include "blauPalette.h"
#include "gruenPalette.h"

#include "background.h"

#define TILES 16084

//background data
//extern const u16 background_Map[1024];
//extern const u16 background_Palette[256];
//extern const unsigned char  background_Tiles[TILES];

//display mode setting macro
#define SetMode(mode) REG_DISPCNT = (mode)

//buttons register
volatile unsigned int *BUTTONS = (volatile unsigned int *)0x04000130;

//background enable flags
#define BG0_ENABLE 0x100
#define BG1_ENABLE 0x200
#define BG2_ENABLE 0x400
#define BG3_ENABLE 0x800

//background registers
#define REG_BG0HOFS *(volatile u16*)0x4000010
#define REG_BG0VOFS *(volatile u16*)0x4000012
#define REG_BG0CNT  *(volatile u16*)0x4000008
#define BG_COLOR256 0x80
#define CHAR_SHIFT 0x2
#define SCREEN_SHIFT 0x8
#define BG_WRAPAROUND 0x1
#define BGPaletteMem ((u16*)0x5000000)

#define TEXTBG_SIZE_256x256 0x0
#define TEXTBG_SIZE_256x512 0x8000
#define TEXTBG_SIZE_512x256 0x4000
#define TEXTBG_SIZE_512x512 0xC000

#define CharBaseBlock(n)   (((n)*0x4000)+0x6000000)
#define ScreenBaseBlock(n) (((n)*0x800)+0x6000000)

//video status registers
#define REG_DISPCNT *(volatile u16*)0x4000000
#define REG_VCOUNT *(volatile u16*)0x4000006
#define REG_DISPSTAT *(volatile u16*)0x4000004

//sprite registers
#define SpriteMem  ((u16*)0x7000000)
#define SpriteData ((u16*)0x6010000)
#define SpritePal  ((u16*)0x5000200)

//object flags
#define OBJ_MAP_2D 0x0
#define OBJ_MAP_1D 0x40
#define OBJ_ENABLE 0x1000

//various sprite flags
#define ROTATION_FLAG 0x100
#define SIZE_DOUBLE 0x200
#define MODE_NORMAL 0x0
#define MODE_TRANSPARENT 0x400
#define MODE_WINDOWED 0x800
#define MOSAIC 0x1000
#define COLOR_16 0x0
#define COLOR_256 0x2000
#define SQUARE 0x0
#define TALL 0x4000
#define WIDE 0x8000
#define ROTDATA(n) ((n) << 9)
#define HORIZONTAL_FLIP 0x1000
#define VERTICAL_FLIP 0x2000
#define SIZE_8 0x0
#define SIZE_16 0x4000
#define SIZE_32 0x8000
#define SIZE_64 0xC000

#define PRIORITY(n) ( (n) << 10 )
#define PALETTE(n) ( (n) << 12 )

//sprite attributes struct
typedef struct tagSprite {
	u16 attribute0;
	u16 attribute1;
	u16 attribute2;
	u16 attribute3;
}Sprite, *pSprite;

Sprite sprites[128];

//gba buttons
#define BTN_UP 64
#define BTN_DOWN 128
#define BTN_LEFT 32
#define BTN_RIGHT 16

void WaitForVBlank(void);
void UpdateSpriteMemory(void);

//defines needed by DMAFastCopy
#define REG_DMA3SAD *(volatile unsigned int*)0x40000D4
#define REG_DMA3DAD *(volatile unsigned int*)0x40000D8
#define REG_DMA3CNT *(volatile unsigned int*)0x40000DC
#define DMA_ENABLE 0x80000000
#define DMA_TIMING_IMMEDIATE 0x00000000
#define DMA_16 0x00000000
#define DMA_32 0x04000000
#define DMA_32NOW (DMA_ENABLE | DMA_TIMING_IMMEDIATE | DMA_32)
#define DMA_16NOW (DMA_ENABLE | DMA_TIMING_IMMEDIATE | DMA_16)

void DMAFastCopy(void*, void*, unsigned int, unsigned int);


int main(void) {
	signed short x = 88, y = 48;
	signed short xinc = 1;

	int bgx = 0, bgy = 0;

	u16* bg0map = (u16*)ScreenBaseBlock(31);

	REG_BG0CNT = (TEXTBG_SIZE_256x256 | BG_COLOR256 | (31 << SCREEN_SHIFT) | BG_WRAPAROUND);

	int char_number = 0;

	int curPalette = 0;
	int prevPalette = curPalette;

	int frames = 0;

	//set video mode 0 enable objects and map them 1D in memory
	SetMode(0 | BG0_ENABLE | OBJ_ENABLE | OBJ_MAP_1D);

	//copy background data
	DMAFastCopy( (void*)backgroundPal,      (void*)BGPaletteMem,     256,        DMA_16NOW );
	DMAFastCopy( (void*)backgroundMap,      (void*)bg0map,           512,        DMA_32NOW );
	DMAFastCopy( (void*)backgroundTiles,    (void*)CharBaseBlock(0), TILES / 4 , DMA_32NOW );

	//set all sprites to be at bottom right corner
	for (int n = 0; n < 128; n++) {
		sprites[n].attribute0 = 160;
		sprites[n].attribute1 = 240;
	}

	//copy sprites palette
	DMAFastCopy( (void*)spherePalette, (void*)SpritePal, 256, DMA_16NOW );
	//copy sprites data
	DMAFastCopy( (void*)sphereData, (void*)SpriteData, 256 * 8, DMA_16NOW );

	//give it attributes
	sprites[0].attribute0 = COLOR_256 | y;
	sprites[0].attribute1 = SIZE_64 | x;
	sprites[0].attribute2 = char_number;


	while(1) {
		//set its x coord
		sprites[0].attribute1 = SIZE_64 | x;

		UpdateSpriteMemory();
		WaitForVBlank();

		REG_BG0VOFS = bgy;
		REG_BG0HOFS = bgx;

		frames++;
		if (frames == 20) {

			//check if buttons were pressed
			if (*BUTTONS) {
				//horiz movement
				if (!(*BUTTONS & BTN_LEFT) && x >= 0) x -= xinc;
				else if (!(*BUTTONS & BTN_RIGHT) && (x + sphere_WIDTH) <= 240) x += xinc;

				//switch palette var
				if (!(*BUTTONS & BTN_UP)) curPalette = 1;
				else if (!(*BUTTONS & BTN_DOWN)) curPalette = 2;
				else curPalette = 0;
			}
			frames = 0;
		}
		if (frames == 40) 
//			bgx++;
		if (frames == 80) {
//			bgy++;
//			frames = 0;
		}

		//check if palette var was changed to avoid copying same stuff over and over again
		if (curPalette != prevPalette) {
			switch (curPalette) {
				case 0:
					DMAFastCopy( (void*)sphereData, (void*)SpriteData, 256 * 8, DMA_16NOW );
					break;
				case 1:
					DMAFastCopy( (void*)blauSphereData, (void*)SpriteData, 256 * 8, DMA_16NOW );
					break;
				case 2:
					DMAFastCopy( (void*)gruenSphereData, (void*)SpriteData, 256 * 8, DMA_16NOW );
					break;
			}
		}

		prevPalette = curPalette;

	}

	return 0;
}

void WaitForVBlank(void) { while((REG_DISPSTAT & 1)); }

void UpdateSpriteMemory(void) {
	u16* tmp;
	tmp = (u16*)sprites;
	DMAFastCopy( (void*)tmp, (void*)SpriteMem, 128*4, DMA_16NOW );
}

void DMAFastCopy(void* source, void* dest, unsigned int count, unsigned int mode) {
	if (mode == DMA_16NOW || mode == DMA_32NOW) {
		REG_DMA3SAD = (unsigned int) source;
		REG_DMA3DAD = (unsigned int) dest;
		REG_DMA3CNT = count | mode;
	}
}
