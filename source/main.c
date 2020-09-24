//SPDX-License-Identifier: BSD-3-Clause
//SPDX-FileCopyrightText: 2020 Lorenzo Cauli (lorecast162)

#include <tonc.h>

//background data header
#include "background.h"

//include spheres data and palette
#define sphere_WIDTH 64
#define sphere_HEIGHT 64
#include "rotSphere.h"
#include "blauSphere.h"
#include "gruenSphere.h"
#include "sprites.h"

#define CharBaseBlock(n)   (void*)(tile_mem[n])
#define ScreenBaseBlock(n) (void*)(se_mem[n])

int main(void) {
	//sprite coordinates and x inc variable to regulate sprite speed
	signed short x = 88, y = 48;
	signed short xinc = 2;

	//sprite attributes variable. will use this as buffer
	OBJ_ATTR sprite;

	//background coordinates. used for scrolling
	int bgx = 0, bgy = 0;

	//background tilemap pointer
	u16* bg0map = (u16*)ScreenBaseBlock(31);

	//set background 0 control register to use 32x32 tiles (32 tiles wide, 32 tiles high), use 8bit color and use screen base block 31
	REG_BG0CNT = (BG_REG_32x32 | BG_8BPP | BG_SBB(31) );

	//this is our sprite's number
	int char_number = 0;

	//used to switch palette without doing it every frame
	int curPalette = 0;
	int prevPalette = curPalette;

	//frame counter
	u16 frames = 0;

	//NULL out OAM
	OAM_CLEAR();

	//set video mode 0, use background 0, enable objects and map them 1D in memory
	REG_DISPCNT = (DCNT_MODE0 | DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D);

	//copy background data
	//palette
	DMA_TRANSFER( (void*)MEM_PAL_BG,		(void*)backgroundPal,      backgroundPalLen,			3,	DMA_16NOW );
	//tilemap
	DMA_TRANSFER( (void*)bg0map,			(void*)backgroundMap,      backgroundMapLen, 			3,	DMA_32NOW );
	//raw tiles data
	DMA_TRANSFER( (void*)CharBaseBlock(0),	(void*)backgroundTiles,    backgroundTilesLen / 4, 		3,	DMA_32NOW );

	//copy sprites palette
	DMA_TRANSFER( MEM_PAL_OBJ, (void*)spritesPal, spritesPalLen/2, 3, DMA_16NOW );
	//copy sprites data
	DMA_TRANSFER( MEM_VRAM_OBJ, (void*)rotSphereTiles, rotSphereTilesLen / 2, 3, DMA_16NOW );

	//set sprite attrs
	//use 8 bit color and set y coords
	sprite.attr0 = ATTR0_8BPP | (y & 0x00FF);
	//use 64x64 pixel sprites and set x coords
	sprite.attr1 = ATTR1_SIZE_64 | (x & 0x00FF);
	//set sprite to our character number
	sprite.attr2 = char_number;

	while(1) {
		//update sprite x coord
		sprite.attr1 = ATTR1_SIZE_64 | x;

		//wait for vertical sync
		vid_vsync();

		//copy sprite to OAM
		oam_copy(oam_mem, &sprite, 1);	

		//set background offsets
		REG_BG0VOFS = bgy;
		REG_BG0HOFS = bgx;

			//check if buttons were pressed
			if (REG_KEYS) {
				//horiz movement
				if (!(REG_KEYS & KEY_LEFT) && x >= 0) x -= xinc;
				else if (!(REG_KEYS & KEY_RIGHT) && (x + sphere_WIDTH) <= 240) x += xinc;

				//switch palette
				if (!(REG_KEYS & KEY_UP)) curPalette = 1;
				else if (!(REG_KEYS & KEY_DOWN)) curPalette = 2;
				else curPalette = 0;
			}
			//increment bg x scroll every frame
			bgx++;
			//increment bg y scroll every 2 frames
			if (frames % 2 == 0 )bgy++;

		//check if palette var was changed to avoid copying same stuff over and over again
		if (curPalette != prevPalette) {
			switch (curPalette) {
				case 0:
					DMA_TRANSFER( MEM_VRAM_OBJ, (void*)rotSphereTiles,		rotSphereTilesLen / 2,  3, DMA_16NOW );
					break;
				case 1:
					DMA_TRANSFER( MEM_VRAM_OBJ, (void*)blauSphereTiles,	blauSphereTilesLen / 2,  3, DMA_16NOW );
					break;
				case 2:
					DMA_TRANSFER( MEM_VRAM_OBJ, (void*)gruenSphereTiles,	gruenSphereTilesLen / 2,  3, DMA_16NOW );
					break;
			}
		}

		//assign current palette to previous for above check
		prevPalette = curPalette;
		
		//increment frame counter
		frames++;
	}

	return 0;
}
