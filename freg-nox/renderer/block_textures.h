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
#ifndef BLOCK_TEXTURES_H
#define BLOCK_TEXTURES_H
#include "../header.h"

/*normals - defined in renderer.h*/
#ifndef WORLD_NORMAL_X
#define WORLD_NORMAL_X      0
#define WORLD_NORMAL_MIN_X  1
#define WORLD_NORMAL_Y      2
#define WORLD_NORMAL_MIN_Y  3
#define WORLD_NORMAL_Z      4
#define WORLD_NORMAL_MIN_Z  5
#endif

#define R_MAX_SUB 32
#define R_MAX_SUB_LOG2 5
#define R_MAX_KIND 16
#define R_MAX_KIND_LOG2 4

#define R_MAX_TEXTURE_SCALE 8
#define R_DEFAULT_TEXTURE_SCALE 8

extern unsigned char texture_table[];
extern unsigned char texture_scale_table[];


inline unsigned char rGetBlockTexture( subs sub, kinds kind, unsigned char normal_id )
{
    return texture_table[ normal_id  |
                          ( sub << 3 ) |
                          ( kind << ( R_MAX_SUB_LOG2 + 3 ) )  ];
}


//возвращает масштаб текстуры. поддерживаются размеры 1, 2, 4, 8.
//размеры не кратные степени 2 пока что не поддерживаются.
inline unsigned char rGetBlockTextureScale( unsigned char texture_id )
{
    return texture_scale_table[ texture_id ];
}


void r_InitTextureTable();
#endif//BLOCK_TEXTURES_H
