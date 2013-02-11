/*
*This file is part of FREG.
*
*FREG is free software: you can redistribute it and/or modify
*it under the terms of the GNU General Public License as published by
*the Free Software Foundation, either version 3 of the License, or
*(at your option) any later version.
*
*FREG is distributed in the hope that it will be useful,
*but WITHOUT ANY WARRANTY; without even the implied warranty of
*MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*GNU General Public License for more details.
*
*You should have received a copy of the GNU General Public License
*along with FREG. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef BLOCK_TEXTURES_CPP
#define BLOCK_TEXTURES_CPP
#include "block_textures.h"

unsigned char texture_table[ R_MAX_SUB * R_MAX_KIND * 8 ];
unsigned char texture_scale_table[ 256 ];

void r_InitTextureTable()
{
    for( int i=0; i< R_MAX_SUB; i++ )
    {
        for( int j=0; j< R_MAX_KIND; j++ )
        {
            for( int k=0; k<8; k++ )
             texture_table[ k  |( i << 3 ) | ( j << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 31;
        }
    }

    for( int i= 0; i< 256; i++ )
        texture_scale_table[i]= R_DEFAULT_TEXTURE_SCALE;

    //block stone   subs-----kinds
    texture_table[ 0  |( STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 0;
    texture_table[ 1  |( STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 0;
    texture_table[ 2  |( STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 0;
    texture_table[ 3  |( STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 0;
    texture_table[ 4  |( STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 0;
    texture_table[ 5  |( STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 0;
    texture_scale_table[0]= 1;


    //block moss stone
    texture_table[ 0  |( MOSS_STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 1;
    texture_table[ 1  |( MOSS_STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 1;
    texture_table[ 2  |( MOSS_STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 1;
    texture_table[ 3  |( MOSS_STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 1;
    texture_table[ 4  |( MOSS_STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 1;
    texture_table[ 5  |( MOSS_STONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 1;


    //block null stone
    texture_table[ 0  |( NULLSTONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 2;
    texture_table[ 1  |( NULLSTONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 2;
    texture_table[ 2  |( NULLSTONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 2;
    texture_table[ 3  |( NULLSTONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 2;
    texture_table[ 4  |( NULLSTONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 2;
    texture_table[ 5  |( NULLSTONE << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 2;
	texture_scale_table[2]= 1;

    //block sky
    texture_table[ 0  |( SKY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 3;
    texture_table[ 1  |( SKY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 3;
    texture_table[ 2  |( SKY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 3;
    texture_table[ 3  |( SKY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 3;
    texture_table[ 4  |( SKY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 3;
    texture_table[ 5  |( SKY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 3;

    //block star
    texture_table[ 0  |( STAR << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 4;
    texture_table[ 1  |( STAR << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 4;
    texture_table[ 2  |( STAR << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 4;
    texture_table[ 3  |( STAR << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 4;
    texture_table[ 4  |( STAR << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 4;
    texture_table[ 5  |( STAR << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 4;

    //block sun moon
    texture_table[ 0  |( SUN_MOON << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 5;
    texture_table[ 1  |( SUN_MOON << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 5;
    texture_table[ 2  |( SUN_MOON << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 5;
    texture_table[ 3  |( SUN_MOON << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 5;
    texture_table[ 4  |( SUN_MOON << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 5;
    texture_table[ 5  |( SUN_MOON << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 5;

    //block soil
    texture_table[ 0  |( SOIL << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 6;
    texture_table[ 1  |( SOIL << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 6;
    texture_table[ 2  |( SOIL << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 6;
    texture_table[ 3  |( SOIL << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 6;
    texture_table[ 4  |( SOIL << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 6;
    texture_table[ 5  |( SOIL << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 6;

    //dwarf
    texture_table[ 0  |( H_MEAT << 3 ) | ( DWARF << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 7;
    texture_table[ 1  |( H_MEAT << 3 ) | ( DWARF << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 7;
    texture_table[ 2  |( H_MEAT << 3 ) | ( DWARF << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 7;
    texture_table[ 3  |( H_MEAT << 3 ) | ( DWARF << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 7;
    texture_table[ 4  |( H_MEAT << 3 ) | ( DWARF << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 7;
    texture_table[ 5  |( H_MEAT << 3 ) | ( DWARF << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 7;
    texture_scale_table[7]= 8;

    //rabbit
    texture_table[ 0  |( A_MEAT << 3 ) | ( RABBIT << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 8;
    texture_table[ 1  |( A_MEAT << 3 ) | ( RABBIT << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 8;
    texture_table[ 2  |( A_MEAT << 3 ) | ( RABBIT << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 8;
    texture_table[ 3  |( A_MEAT << 3 ) | ( RABBIT << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 8;
    texture_table[ 4  |( A_MEAT << 3 ) | ( RABBIT << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 8;
    texture_table[ 5  |( A_MEAT << 3 ) | ( RABBIT << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 8;

    //grass
    texture_table[ 0  |( GREENERY << 3 ) | ( GRASS << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 9;
    texture_table[ 1  |( GREENERY << 3 ) | ( GRASS << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 9;
    texture_table[ 2  |( GREENERY << 3 ) | ( GRASS << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 9;
    texture_table[ 3  |( GREENERY << 3 ) | ( GRASS << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 9;
    texture_table[ 4  |( GREENERY << 3 ) | ( GRASS << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 10;
    texture_table[ 5  |( GREENERY << 3 ) | ( GRASS << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 10;

    //leafs
    texture_table[ 0  |( GREENERY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 15;
    texture_table[ 1  |( GREENERY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 15;
    texture_table[ 2  |( GREENERY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 15;
    texture_table[ 3  |( GREENERY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 15;
    texture_table[ 4  |( GREENERY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 15;
    texture_table[ 5  |( GREENERY << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 15;

    //wood
    texture_table[ 0  |( WOOD << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 11;
    texture_table[ 1  |( WOOD << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 11;
    texture_table[ 2  |( WOOD << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 11;
    texture_table[ 3  |( WOOD << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 11;
    texture_table[ 4  |( WOOD << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 12;
    texture_table[ 5  |( WOOD << 3 ) | ( BLOCK << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 12;


    //water
    texture_table[ 0  |( WATER << 3 ) | ( LIQUID << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 14;
    texture_table[ 1  |( WATER << 3 ) | ( LIQUID << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 14;
    texture_table[ 2  |( WATER << 3 ) | ( LIQUID << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 14;
    texture_table[ 3  |( WATER << 3 ) | ( LIQUID << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 14;
    texture_table[ 4  |( WATER << 3 ) | ( LIQUID << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 14;
    texture_table[ 5  |( WATER << 3 ) | ( LIQUID << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 14;

     //bush
    texture_table[ 0  |( WOOD << 3 ) | ( BUSH << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 11;
    texture_table[ 1  |( WOOD << 3 ) | ( BUSH << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 11;
    texture_table[ 2  |( WOOD << 3 ) | ( BUSH << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 11;
    texture_table[ 3  |( WOOD << 3 ) | ( BUSH << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 11;
    texture_table[ 4  |( WOOD << 3 ) | ( BUSH << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 13;
    texture_table[ 5  |( WOOD << 3 ) | ( BUSH << ( R_MAX_SUB_LOG2 + 3 ) )  ]= 13;
}


#endif//BLOCK_TEXTURES_CPP
