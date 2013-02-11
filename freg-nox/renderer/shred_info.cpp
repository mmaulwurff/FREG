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
#ifndef SHRED_INFO_CPP
#define SHRED_INFO_CPP
#include "ph.h"
#include "renderer.h"
#include "block_textures.h"

void r_ShredInfo::UpdateCube( short x0, short y0, short z0, short x1, short y1, short z1 )
{
    unsigned char* inf;
    short x, y, z;
    for( x= x0; x<= x1; x++ )
        for( y= y0; y<= y1; y++ )
        {
            inf= &visibly_information[ (x<<11) | (y<<7) | z0 ];
            for( z= z0; z<= z1; z++ )
            {
                //b= shred->GetBlock( x, y, z );
                // if( b!= NULL )
                inf[0]= shred->GetBlock( x, y, z )->Block::Transparent();
                // else
                //     inf[0]= 2;
                inf++;
            }
        }

    updated= true;
    if( x0 == 0 && west_shred != NULL )
        west_shred->updated= true;
    if( y0 == 0 && north_shred != NULL )
        north_shred->updated= true;
}

void r_Renderer::UpdateCube( short x0, short y0, short z0, short x1, short y1, short z1 )
{
  //  while( ! renderer_initialized  )
     //  usleep( 1000 );
    host_data_mutex.lock();

    short X0, Y0, X1, Y1;

    X0= x0>>4;
    X1= x1>>4;
    Y0= y0>>4;
    Y1= y1>>4;
    short x, y, i0, j0, i1, j1;

    r_ShredInfo* shred;
    for( x= X0, i0= x0&15, i1= 15; x<= X1; x++ )
    {
        for( y= Y0, j0= y0&15, j1= 15; y<= Y1; y++ )
        {
            if( x == X1 )
                i1= x1&15;
            if( y == Y1 )
                j1= y1&15;
            shred= &shreds[ x + y * visibly_world_size[0] ];
            shred->UpdateCube( i0, j0, z0, i1, j1, z1 );
            j0 = 0;
        }
        i0= 0;
    }

    host_data_mutex.unlock();
}

void r_Renderer::UpdateShred( short x, short y )
{
    r_ShredInfo* shred= &shreds[ x + y * visibly_world_size[0] ];
    shred->UpdateShred();
}

void r_Renderer::UpdateBlock( short x, short y, short z )
{
   // while( ! renderer_initialized )
   //     usleep( 1000 );

    if( z == ( R_SHRED_HEIGHT - 1 ) )
        return;//ignore sky

    host_data_mutex.lock();

    short X, Y;

    X= x>>4;
    Y= y>>4;
    x&=15;
    y&=15;

    register r_ShredInfo* shred= &shreds[ X + Y * visibly_world_size[0] ];

    shred->updated= true;

    if( x == 0 && shred->west_shred != NULL )
        shred->west_shred->updated= true;

    if( y == 0 && shred->north_shred != NULL )
        shred->north_shred->updated= true;

    Block* b= shred->shred->GetBlock( x, y, z );
    unsigned char inf;
    // if( b != NULL )
    inf= b->Block::Transparent();
    //else
    //  inf = 2;

    shred->visibly_information[ (x<<11) | (y<<7) | z ]= inf;

    host_data_mutex.unlock();
}

void r_ShredInfo::GetVisiblyInformation()
{
    //false видим
    //true невидим
    Block* b;
    unsigned int x, y, z;

    unsigned char* inf= visibly_information;
    for( x= 0; x< R_SHRED_WIDTH; x++ )
    {
        for( y= 0; y< R_SHRED_WIDTH; y++ )
        {
            for( z= 0; z< R_SHRED_HEIGHT; z++ )
            {
                b= shred->GetBlock( x, y, z );
                // if( b != NULL )
                inf[0]= b->Block::Transparent();
                // else
                //    inf[0]= 2;
                //if( inf[0] == 1 )inf[0]= 0;//для полупрозрачных блоков

                inf++;
            }
            inf +=( 128 - R_SHRED_HEIGHT );//компенсация различий высоты (реальной и в массиве)
        }
    }
}



void r_ShredInfo::GetQuadCount()
{
    unsigned int x, y, z;
    unsigned int result= 0;

    unsigned char x1= false , y1= false , z1= false;
    unsigned char current= 0;
    //false видим
    //true невидим
    max_geometry_height= 0;
    min_geometry_height= 128;

    for( x= 0; x< R_SHRED_WIDTH  - 1 ; x++ )
    {
        for( y= 0; y< R_SHRED_WIDTH  - 1; y++ )
        {
            for( z= 0; z< R_SHRED_HEIGHT  - 2; z++ )
            {
                current= z1;

                x1= visibly_information[ ((x+1)<<11) | (y<<7) | z ];
                y1= visibly_information[ (x<<11) | ((y+1)<<7) | z ];
                z1= visibly_information[ (x<<11) | (y<<7) | (z+1) ];

                if( current != x1 )
                {
                    max_geometry_height= max( z, max_geometry_height );
                    min_geometry_height= min( z, min_geometry_height );
                    result ++;
                }
                if( current != y1 )
                {
                    max_geometry_height= max( z, max_geometry_height );
                    min_geometry_height= min( z, min_geometry_height );
                    result ++;
                }
                if( current != z1 )
                {
                    max_geometry_height= max( z, max_geometry_height );
                    min_geometry_height= min( z, min_geometry_height );
                    result ++;
                }
            }
            z1= false;
        }
    }


    /*обход южной границы*/
    y= R_SHRED_WIDTH  - 1;
    for( x= 0; x< R_SHRED_WIDTH  - 1; x++ )
    {
        z1= false;
        for( z=0; z< R_SHRED_HEIGHT  - 2; z++ )
        {
            current= z1;

            x1= visibly_information[ ((x+1)<<11) | (y<<7) | z ];
            if( south_shred != NULL )
                y1= south_shred->visibly_information[ (x<<11) | z ];
            else
                y1= current;
            z1= visibly_information[ (x<<11) | (y<<7) | (z+1) ];

            if( current != x1 )
            {
                max_geometry_height= max( z, max_geometry_height );
                min_geometry_height= min( z, min_geometry_height );
                result ++;
            }
            if( current != y1 )
            {
                max_geometry_height= max( z, max_geometry_height );
                min_geometry_height= min( z, min_geometry_height );
                result ++;
            }
            if( current != z1 )
            {
                max_geometry_height= max( z, max_geometry_height );
                min_geometry_height= min( z, min_geometry_height );
                result ++;
            }

        }

    }

    /*обход восточной границы*/
    x= R_SHRED_WIDTH  - 1;
    for( y= 0; y< R_SHRED_WIDTH  - 1; y++ )
    {
        z1= false;
        for( z=0; z< R_SHRED_HEIGHT  - 2; z++ )
        {
            current= z1;
            if( east_shred != NULL )
                x1= east_shred->visibly_information[ (y<<7) | z ];
            else
                x1= current;
            y1= visibly_information[ (x<<11) | ((y+1)<<7) | z ];
            z1= visibly_information[ (x<<11) | (y<<7) | (z+1) ];

            if( current != x1 )
            {
                max_geometry_height= max( z, max_geometry_height );
                min_geometry_height= min( z, min_geometry_height );
                result ++;
            }
            if( current != y1 )
            {
                max_geometry_height= max( z, max_geometry_height );
                min_geometry_height= min( z, min_geometry_height );
                result ++;
            }
            if( current != z1 )
            {
                max_geometry_height= max( z, max_geometry_height );
                min_geometry_height= min( z, min_geometry_height );
                result ++;
            }

        }

    }
    /*обход юговосточного угла*/

    z1= false;
    y = x= R_SHRED_WIDTH  - 1;
    for( z=0; z< R_SHRED_HEIGHT  - 2; z++ )
    {
        current= z1;
        if( east_shred != NULL )
            x1= east_shred->visibly_information[ (y<<7) | z ];
        else
            x1= current;
        if( south_shred != NULL )
            y1= south_shred->visibly_information[ (x<<11) | z ];
        else
            y1= current;
        z1= visibly_information[ (x<<11) | (y<<7) | (z+1) ];

        if( current != x1 )
        {
            max_geometry_height= max( z, max_geometry_height );
            min_geometry_height= min( z, min_geometry_height );
            result ++;
        }
        if( current != y1 )
        {
            max_geometry_height= max( z, max_geometry_height );
            min_geometry_height= min( z, min_geometry_height );
            result ++;
        }
        if( current != z1 )
        {
            max_geometry_height= max( z, max_geometry_height );
            min_geometry_height= min( z, min_geometry_height );
            result ++;
        }

    }

    //max_geometry_height= 97;
    //min_geometry_height= 0;
    quad_count= result;
}



void r_ShredInfo::BuildShred( r_WorldVertex* shred_vertices )
{
    unsigned char x, y, z;
    Block* b;
    unsigned char x1= false , y1= false , z1= false;
    unsigned char current= false;
    //false видим
    //true невидим
    unsigned char normal_id;
    unsigned char light;
    unsigned char tex_id, tex_scale, tc_x, tc_y;

    unsigned int v13[2];
    short X= latitude * 16, Y= longitude * 16;
    for( x= 0; x< R_SHRED_WIDTH  - 1 ; x++ )
    {
        for( y= 0; y< R_SHRED_WIDTH  - 1; y++ )
        {
            z1= visibly_information[ (x<<11) | (y<<7) | min_geometry_height ];
            for( z= min_geometry_height; z<= max_geometry_height; z++ )
            {
                current= z1;

                x1= visibly_information[ ((x+1)<<11) | (y<<7) | z ];
                y1= visibly_information[ (x<<11) | ((y+1)<<7) | z ];
                z1= visibly_information[ (x<<11) | (y<<7) | (z+1) ];

                if( current != x1 )
                {
                    normal_id= WORLD_NORMAL_X;
                    if( current > x1 )
                    {
                        v13[0]= 1;
                        v13[1]= 3;
                        normal_id++;
                        b= shred->GetBlock( x + 1, y, z );
                        // light= shred->LightMap( x + 1, y, z );
                        light= shred->LightMap( x , y, z );
                    }
                    else
                    {
                        v13[0]= 3;
                        v13[1]= 1;
                        b= shred->GetBlock( x, y, z );
                        //light= shred->LightMap( x , y, z );
                        light= shred->LightMap( x + 1, y, z );
                    }

                    tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
                    tex_scale= rGetBlockTextureScale( tex_id );
                    tc_x= ( -y * tex_scale )&7;
                    tc_y= ( z * tex_scale )&7;

                    shred_vertices[ v13[1] ].coord[0]= x + X;
                    shred_vertices[ v13[1] ].coord[1]= y + Y;
                    shred_vertices[ v13[1] ].coord[2]= z;
                    shred_vertices[ v13[1] ].tex_coord[0]= tex_id;
                    shred_vertices[ v13[1] ].tex_coord[1]= ( tc_y + tex_scale ) | ( tc_x << 4 );

                    shred_vertices[2].coord[0]= x + X;
                    shred_vertices[2].coord[1]= y - 1 + Y;
                    shred_vertices[2].coord[2]= z;
                    shred_vertices[2].tex_coord[0]= tex_id;
                    shred_vertices[2].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 ); //1

                    shred_vertices[ v13[0] ].coord[0]= x + X;
                    shred_vertices[ v13[0] ].coord[1]= y - 1 + Y;
                    shred_vertices[ v13[0] ].coord[2]= z - 1;
                    shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
                    shred_vertices[ v13[0] ].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 );//3

                    shred_vertices[0].coord[0]= x + X;
                    shred_vertices[0].coord[1]= y + Y;
                    shred_vertices[0].coord[2]= z - 1;
                    shred_vertices[0].tex_coord[0]= tex_id;
                    shred_vertices[0].tex_coord[1]= tc_y | ( tc_x << 4 );//2

                    shred_vertices[3].normal_id=
                        shred_vertices[2].normal_id=
                            shred_vertices[1].normal_id=
                                shred_vertices[0].normal_id= normal_id;

                    shred_vertices[3].light=
                        shred_vertices[2].light=
                            shred_vertices[1].light=
                                shred_vertices[0].light= light;
                    shred_vertices+= 4;
                }
                if( current != y1 )
                {
                    normal_id= WORLD_NORMAL_Y;
                    if( current > y1 )
                    {
                        v13[0]= 1;
                        v13[1]= 3;
                        normal_id++;
                        b= shred->GetBlock( x, y + 1, z );
                        light= shred->LightMap( x, y, z );
                        //light= shred->LightMap( x, y + 1, z );
                    }
                    else
                    {
                        v13[0]= 3;
                        v13[1]= 1;
                        b= shred->GetBlock( x, y, z );
                        light= shred->LightMap( x, y+1, z );
                        //light= shred->LightMap( x, y, z );
                    }


                    tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
                    tex_scale= rGetBlockTextureScale( tex_id );
                    tc_x= ( x * tex_scale )&7;
                    tc_y= ( z * tex_scale )&7;


                    shred_vertices[0].coord[0]= x + X;
                    shred_vertices[0].coord[1]= y + Y;
                    shred_vertices[0].coord[2]= z;
                    shred_vertices[0].tex_coord[0]= tex_id;
                    shred_vertices[0].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 );//3


                    shred_vertices[ v13[0] ].coord[0]= x - 1 + X;
                    shred_vertices[ v13[0] ].coord[1]= y + Y;
                    shred_vertices[ v13[0] ].coord[2]= z;
                    shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
                    shred_vertices[ v13[0] ].tex_coord[1]= (tc_y + tex_scale ) | ( tc_x << 4 );//2

                    shred_vertices[2].coord[0]= x - 1 + X;
                    shred_vertices[2].coord[1]= y + Y;
                    shred_vertices[2].coord[2]= z - 1;
                    shred_vertices[2].tex_coord[0]= tex_id ;
                    shred_vertices[2].tex_coord[1]= tc_y | ( tc_x << 4 );//0

                    shred_vertices[ v13[1] ].coord[0]= x + X;
                    shred_vertices[ v13[1] ].coord[1]= y + Y;
                    shred_vertices[ v13[1] ].coord[2]= z - 1;
                    shred_vertices[ v13[1] ].tex_coord[0]= tex_id;
                    shred_vertices[ v13[1] ].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 ); //1


                    shred_vertices[3].normal_id=
                        shred_vertices[2].normal_id=
                            shred_vertices[1].normal_id=
                                shred_vertices[0].normal_id= normal_id;

                    shred_vertices[3].light=
                        shred_vertices[2].light=
                            shred_vertices[1].light=
                                shred_vertices[0].light= light;
                    shred_vertices+= 4;
                }
                if( current != z1 )
                {
                    normal_id= WORLD_NORMAL_Z;
                    if( current > z1 )
                    {
                        v13[0]= 1;
                        v13[1]= 3;
                        normal_id++;
                        b= shred->GetBlock( x, y, z + 1 );
                        light= shred->LightMap( x , y, z );
                        //light= shred->LightMap( x, y, z + 1 );
                    }
                    else
                    {
                        v13[0]= 3;
                        v13[1]= 1;
                        b= shred->GetBlock( x, y, z );
                        light= shred->LightMap( x, y, z + 1 );
                        //light= shred->LightMap( x, y, z );
                    }

                    tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
                    tex_scale= rGetBlockTextureScale( tex_id );
                    tc_x= ( x * tex_scale )&7;
                    tc_y= ( -y * tex_scale )&7;

                    shred_vertices[0].coord[0]= x + X;
                    shred_vertices[0].coord[1]= y + Y;
                    shred_vertices[0].coord[2]= z;
                    shred_vertices[0].tex_coord[0]= tex_id;
                    shred_vertices[0].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 ); //1

                    shred_vertices[ v13[0] ].coord[0]= x + X;
                    shred_vertices[ v13[0] ].coord[1]= y - 1 + Y;
                    shred_vertices[ v13[0] ].coord[2]= z;
                    shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
                    shred_vertices[ v13[0] ].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 );//3


                    shred_vertices[2].coord[0]= x - 1 + X;
                    shred_vertices[2].coord[1]= y - 1 + Y;
                    shred_vertices[2].coord[2]= z;
                    shred_vertices[2].tex_coord[0]= tex_id;
                    shred_vertices[2].tex_coord[1]= ( tc_y + tex_scale ) | ( tc_x << 4 );//3

                    shred_vertices[ v13[1] ].coord[0]= x - 1 + X;
                    shred_vertices[ v13[1] ].coord[1]= y + Y;
                    shred_vertices[ v13[1] ].coord[2]= z;
                    shred_vertices[ v13[1] ].tex_coord[0]= tex_id ;
                    shred_vertices[ v13[1] ].tex_coord[1]= tc_y | ( tc_x << 4 );//2


                    shred_vertices[3].normal_id=
                        shred_vertices[2].normal_id=
                            shred_vertices[1].normal_id=
                                shred_vertices[0].normal_id= normal_id;

                    shred_vertices[3].light=
                        shred_vertices[2].light=
                            shred_vertices[1].light=
                                shred_vertices[0].light= light;
                    shred_vertices+= 4;
                }
            }
            z1= false;
        }
    }

    //обход южной границы
    y= R_SHRED_WIDTH - 1;
    for( x= 0; x< R_SHRED_WIDTH - 1; x++ )
    {
        z1= visibly_information[ (x<<11) | (y<<7) | min_geometry_height ];
        for( z= min_geometry_height; z<= max_geometry_height; z++ )
        {
            current= z1;
            x1= visibly_information[ ((x+1)<<11) | (y<<7) | z ];
            if( south_shred != NULL )
                y1= south_shred->visibly_information[ (x<<11) | z ];
            else
                y1= current;
            z1= visibly_information[ (x<<11) | (y<<7) | (z+1) ];

            if( current != x1 )
            {
                normal_id= WORLD_NORMAL_X;
                if( current > x1 )
                {
                    v13[0]= 1;
                    v13[1]= 3;
                    normal_id++;
                    b= shred->GetBlock( x + 1, y, z );
                    light= shred->LightMap( x , y, z );
                    //light= shred->LightMap( x + 1, y, z );
                }
                else
                {
                    v13[0]= 3;
                    v13[1]= 1;
                    b= shred->GetBlock( x, y, z );
                    light= shred->LightMap( x + 1, y, z );
                    //light= shred->LightMap( x, y, z );
                }


                tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
                tex_scale= rGetBlockTextureScale( tex_id );
                tc_x= ( -y * tex_scale )&7;
                tc_y= ( z * tex_scale )&7;

                shred_vertices[ v13[1] ].coord[0]= x + X;
                shred_vertices[ v13[1] ].coord[1]= y + Y;
                shred_vertices[ v13[1] ].coord[2]= z;
                shred_vertices[ v13[1] ].tex_coord[0]= tex_id;
                shred_vertices[ v13[1] ].tex_coord[1]= ( tc_y + tex_scale ) + ( tc_x << 4 );

                shred_vertices[2].coord[0]= x + X;
                shred_vertices[2].coord[1]= y - 1 + Y;
                shred_vertices[2].coord[2]= z;
                shred_vertices[2].tex_coord[0]= tex_id;
                shred_vertices[2].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 ); //1

                shred_vertices[ v13[0] ].coord[0]= x + X;
                shred_vertices[ v13[0] ].coord[1]= y - 1 + Y;
                shred_vertices[ v13[0] ].coord[2]= z - 1;
                shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
                shred_vertices[ v13[0] ].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 );//3

                shred_vertices[0].coord[0]= x + X;
                shred_vertices[0].coord[1]= y + Y;
                shred_vertices[0].coord[2]= z - 1;
                shred_vertices[0].tex_coord[0]= tex_id;
                shred_vertices[0].tex_coord[1]= tc_y | ( tc_x << 4 );//2

                shred_vertices[3].normal_id=
                    shred_vertices[2].normal_id=
                        shred_vertices[1].normal_id=
                            shred_vertices[0].normal_id= normal_id;

                shred_vertices[3].light=
                    shred_vertices[2].light=
                        shred_vertices[1].light=
                            shred_vertices[0].light= light;
                shred_vertices+= 4;
            }
            if( current != y1 )
            {
                normal_id= WORLD_NORMAL_Y;
                if( current > y1 )
                {
                    v13[0]= 1;
                    v13[1]= 3;
                    normal_id++;
                    b= south_shred->shred->GetBlock( x, 0, z );
                    //light= south_shred->shred->LightMap( x, 0, z );
                    light= shred->LightMap( x, y, z );
                }
                else
                {
                    v13[0]= 3;
                    v13[1]= 1;
                    b= shred->GetBlock( x, y, z );
                    light= south_shred->shred->LightMap( x, 0, z );
                    //light= shred->LightMap( x, y, z );
                }


                tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
                tex_scale= rGetBlockTextureScale( tex_id );
                tc_x= ( x * tex_scale )&7;
                tc_y= ( z * tex_scale )&7;


                shred_vertices[0].coord[0]= x + X;
                shred_vertices[0].coord[1]= y + Y;
                shred_vertices[0].coord[2]= z;
                shred_vertices[0].tex_coord[0]= tex_id;
                shred_vertices[0].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 );//3

                shred_vertices[ v13[0] ].coord[0]= x - 1 + X;
                shred_vertices[ v13[0] ].coord[1]= y + Y;
                shred_vertices[ v13[0] ].coord[2]= z;
                shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
                shred_vertices[ v13[0] ].tex_coord[1]= (tc_y + tex_scale ) | ( tc_x << 4 );//2

                shred_vertices[2].coord[0]= x - 1 + X;
                shred_vertices[2].coord[1]= y + Y;
                shred_vertices[2].coord[2]= z - 1;
                shred_vertices[2].tex_coord[0]= tex_id ;
                shred_vertices[2].tex_coord[1]= tc_y | ( tc_x << 4 );//0

                shred_vertices[ v13[1] ].coord[0]= x + X;
                shred_vertices[ v13[1] ].coord[1]= y + Y;
                shred_vertices[ v13[1] ].coord[2]= z - 1;
                shred_vertices[ v13[1] ].tex_coord[0]= tex_id;
                shred_vertices[ v13[1] ].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 ); //1

                shred_vertices[3].normal_id=
                    shred_vertices[2].normal_id=
                        shred_vertices[1].normal_id=
                            shred_vertices[0].normal_id= normal_id;

                shred_vertices[3].light=
                    shred_vertices[2].light=
                        shred_vertices[1].light=
                            shred_vertices[0].light= light;

                shred_vertices+= 4;
            }
            if( current != z1 )
            {
                normal_id= WORLD_NORMAL_Z;
                if( current > z1 )
                {
                    v13[0]= 1;
                    v13[1]= 3;
                    normal_id++;
                    b= shred->GetBlock( x, y, z + 1 );
                    light= shred->LightMap( x, y, z );
                    //light= shred->LightMap( x, y, z + 1 );
                }
                else
                {
                    v13[0]= 3;
                    v13[1]= 1;
                    b= shred->GetBlock( x, y, z );
                    light= shred->LightMap( x, y, z + 1 );
                    //light= shred->LightMap( x, y, z );
                }


                tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
                tex_scale= rGetBlockTextureScale( tex_id );
                tc_x= ( x * tex_scale )&7;
                tc_y= ( -y * tex_scale )&7;

                shred_vertices[0].coord[0]= x + X;
                shred_vertices[0].coord[1]= y + Y;
                shred_vertices[0].coord[2]= z;
                shred_vertices[0].tex_coord[0]= tex_id;
                shred_vertices[0].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 ); //1

                shred_vertices[ v13[0] ].coord[0]= x + X;
                shred_vertices[ v13[0] ].coord[1]= y - 1 + Y;
                shred_vertices[ v13[0] ].coord[2]= z;
                shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
                shred_vertices[ v13[0] ].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 );//3


                shred_vertices[2].coord[0]= x - 1 + X;
                shred_vertices[2].coord[1]= y - 1 + Y;
                shred_vertices[2].coord[2]= z;
                shred_vertices[2].tex_coord[0]= tex_id;
                shred_vertices[2].tex_coord[1]= ( tc_y + tex_scale ) | ( tc_x << 4 );//3

                shred_vertices[ v13[1] ].coord[0]= x - 1 + X;
                shred_vertices[ v13[1] ].coord[1]= y + Y;
                shred_vertices[ v13[1] ].coord[2]= z;
                shred_vertices[ v13[1] ].tex_coord[0]= tex_id ;
                shred_vertices[ v13[1] ].tex_coord[1]= tc_y | ( tc_x << 4 );//2

                shred_vertices[3].normal_id=
                    shred_vertices[2].normal_id=
                        shred_vertices[1].normal_id=
                            shred_vertices[0].normal_id= normal_id;

                shred_vertices[3].light=
                    shred_vertices[2].light=
                        shred_vertices[1].light=
                            shred_vertices[0].light= light;

                shred_vertices+= 4;
            }
        }
    }


    //обход восточной границы
    x= R_SHRED_WIDTH - 1;
    for( y= 0; y< R_SHRED_WIDTH - 1; y++ )
    {
        z1= visibly_information[ (x<<11) | (y<<7) | min_geometry_height ];
        for( z= min_geometry_height; z<= max_geometry_height; z++ )
        {
            current= z1;
            if( east_shred != NULL )
                x1= east_shred->visibly_information[ (y<<7) | z ];
            else
                x1= current;
            y1= visibly_information[ (x<<11) | ((y+1)<<7) | z ];
            z1= visibly_information[ (x<<11) | (y<<7) | (z+1) ];

            if( current != x1 )
            {
                normal_id= WORLD_NORMAL_X;
                if( current > x1 )
                {
                    v13[0]= 1;
                    v13[1]= 3;
                    normal_id++;
                    b= east_shred->shred->GetBlock( 0, y, z );
                    light= shred->LightMap( x, y, z );
                    //light= east_shred->shred->LightMap( 0, y, z );
                }
                else
                {
                    v13[0]= 3;
                    v13[1]= 1;
                    b= shred->GetBlock( x, y, z );
                    light= east_shred->shred->LightMap( 0, y, z );
                    //light= shred->LightMap( x, y, z );
                }


                tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
                tex_scale= rGetBlockTextureScale( tex_id );
                tc_x= ( -y * tex_scale )&7;
                tc_y= ( z * tex_scale )&7;

                shred_vertices[ v13[1] ].coord[0]= x + X;
                shred_vertices[ v13[1] ].coord[1]= y + Y;
                shred_vertices[ v13[1] ].coord[2]= z;
                shred_vertices[ v13[1] ].tex_coord[0]= tex_id;
                shred_vertices[ v13[1] ].tex_coord[1]= ( tc_y + tex_scale ) + ( tc_x << 4 );

                shred_vertices[2].coord[0]= x + X;
                shred_vertices[2].coord[1]= y - 1 + Y;
                shred_vertices[2].coord[2]= z;
                shred_vertices[2].tex_coord[0]= tex_id;
                shred_vertices[2].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 ); //1

                shred_vertices[ v13[0] ].coord[0]= x + X;
                shred_vertices[ v13[0] ].coord[1]= y - 1 + Y;
                shred_vertices[ v13[0] ].coord[2]= z - 1;
                shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
                shred_vertices[ v13[0] ].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 );//3

                shred_vertices[0].coord[0]= x + X;
                shred_vertices[0].coord[1]= y + Y;
                shred_vertices[0].coord[2]= z - 1;
                shred_vertices[0].tex_coord[0]= tex_id;
                shred_vertices[0].tex_coord[1]= tc_y | ( tc_x << 4 );//2

                shred_vertices[3].normal_id=
                    shred_vertices[2].normal_id=
                        shred_vertices[1].normal_id=
                            shred_vertices[0].normal_id= normal_id;

                shred_vertices[3].light=
                    shred_vertices[2].light=
                        shred_vertices[1].light=
                            shred_vertices[0].light= light;
                shred_vertices+= 4;
            }
            if( current != y1 )
            {
                normal_id= WORLD_NORMAL_Y;
                if( current > y1 )
                {
                    v13[0]= 1;
                    v13[1]= 3;
                    normal_id++;
                    b= shred->GetBlock( x, y + 1, z );
                    light= shred->LightMap( x, y, z );
                    //light= shred->LightMap( x, y + 1, z );
                }
                else
                {
                    v13[0]= 3;
                    v13[1]= 1;
                    b= shred->GetBlock( x, y, z );
                    light= shred->LightMap( x, y + 1, z );
                    //light= shred->LightMap( x, y, z );
                }

                tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
                tex_scale= rGetBlockTextureScale( tex_id );
                tc_x= ( x * tex_scale )&7;
                tc_y= ( z * tex_scale )&7;


                shred_vertices[0].coord[0]= x + X;
                shred_vertices[0].coord[1]= y + Y;
                shred_vertices[0].coord[2]= z;
                shred_vertices[0].tex_coord[0]= tex_id;
                shred_vertices[0].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 );//3


                shred_vertices[ v13[0] ].coord[0]= x - 1 + X;
                shred_vertices[ v13[0] ].coord[1]= y + Y;
                shred_vertices[ v13[0] ].coord[2]= z;
                shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
                shred_vertices[ v13[0] ].tex_coord[1]= (tc_y + tex_scale ) | ( tc_x << 4 );//2

                shred_vertices[2].coord[0]= x - 1 + X;
                shred_vertices[2].coord[1]= y + Y;
                shred_vertices[2].coord[2]= z - 1;
                shred_vertices[2].tex_coord[0]= tex_id ;
                shred_vertices[2].tex_coord[1]= tc_y | ( tc_x << 4 );//0

                shred_vertices[ v13[1] ].coord[0]= x + X;
                shred_vertices[ v13[1] ].coord[1]= y + Y;
                shred_vertices[ v13[1] ].coord[2]= z - 1;
                shred_vertices[ v13[1] ].tex_coord[0]= tex_id;
                shred_vertices[ v13[1] ].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 ); //1

                shred_vertices[3].light=
                    shred_vertices[2].light=
                        shred_vertices[1].light=
                            shred_vertices[0].light= light;

                shred_vertices[3].normal_id=
                    shred_vertices[2].normal_id=
                        shred_vertices[1].normal_id=
                            shred_vertices[0].normal_id= normal_id;
                shred_vertices+= 4;
            }
            if( current != z1 )
            {
                normal_id= WORLD_NORMAL_Z;
                if( current > z1 )
                {
                    v13[0]= 1;
                    v13[1]= 3;
                    normal_id++;
                    b= shred->GetBlock( x, y, z + 1 );
                    light= shred->LightMap( x, y, z );
                    //light= shred->LightMap( x, y, z + 1 );
                }
                else
                {
                    v13[0]= 3;
                    v13[1]= 1;
                    b= shred->GetBlock( x, y, z );
                    light= shred->LightMap( x, y, z + 1 );
                    //light= shred->LightMap( x, y, z );
                }

                tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
                tex_scale= rGetBlockTextureScale( tex_id );
                tc_x= ( x * tex_scale )&7;
                tc_y= ( -y * tex_scale )&7;

                shred_vertices[0].coord[0]= x + X;
                shred_vertices[0].coord[1]= y + Y;
                shred_vertices[0].coord[2]= z;
                shred_vertices[0].tex_coord[0]= tex_id;
                shred_vertices[0].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 ); //1

                shred_vertices[ v13[0] ].coord[0]= x + X;
                shred_vertices[ v13[0] ].coord[1]= y - 1 + Y;
                shred_vertices[ v13[0] ].coord[2]= z;
                shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
                shred_vertices[ v13[0] ].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 );//3


                shred_vertices[2].coord[0]= x - 1 + X;
                shred_vertices[2].coord[1]= y - 1 + Y;
                shred_vertices[2].coord[2]= z;
                shred_vertices[2].tex_coord[0]= tex_id;
                shred_vertices[2].tex_coord[1]= ( tc_y + tex_scale ) | ( tc_x << 4 );//3

                shred_vertices[ v13[1] ].coord[0]= x - 1 + X;
                shred_vertices[ v13[1] ].coord[1]= y + Y;
                shred_vertices[ v13[1] ].coord[2]= z;
                shred_vertices[ v13[1] ].tex_coord[0]= tex_id ;
                shred_vertices[ v13[1] ].tex_coord[1]= tc_y | ( tc_x << 4 );//2

                shred_vertices[3].normal_id=
                    shred_vertices[2].normal_id=
                        shred_vertices[1].normal_id=
                            shred_vertices[0].normal_id= normal_id;

                shred_vertices[3].light=
                    shred_vertices[2].light=
                        shred_vertices[1].light=
                            shred_vertices[0].light= light;
                shred_vertices+= 4;
            }
        }
    }

    //обход юговосточного угла
    z1= visibly_information[ (x<<11) | (y<<7) | min_geometry_height ];
    y= x= R_SHRED_WIDTH - 1;
    for( z= min_geometry_height; z<= max_geometry_height; z++ )
    {
        current= z1;
        if( east_shred != NULL )
            x1= east_shred->visibly_information[ (y<<7) | z ];
        else
            x1= current;
        if( south_shred != NULL )
            y1= south_shred->visibly_information[ (x<<11) | z ];
        else
            y1= current;
        z1= visibly_information[ (x<<11) | (y<<7) | (z+1) ];
        if( current != x1 )
        {
            normal_id= WORLD_NORMAL_X;
            if( current > x1 )
            {
                v13[0]= 1;
                v13[1]= 3;
                normal_id++;
                b= east_shred->shred->GetBlock( 0, y, z );
                light= shred->LightMap( x, y, z );
                //light= east_shred->shred->LightMap( 0, y, z );
            }
            else
            {
                v13[0]= 3;
                v13[1]= 1;
                b= shred->GetBlock( x, y, z );
                light= east_shred->shred->LightMap( 0, y, z );
                //light= shred->LightMap( x, y, z );
            }


            tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
            tex_scale= rGetBlockTextureScale( tex_id );
            tc_x= ( -y * tex_scale )&7;
            tc_y= ( z * tex_scale )&7;

            shred_vertices[ v13[1] ].coord[0]= x + X;
            shred_vertices[ v13[1] ].coord[1]= y + Y;
            shred_vertices[ v13[1] ].coord[2]= z;
            shred_vertices[ v13[1] ].tex_coord[0]= tex_id;
            shred_vertices[ v13[1] ].tex_coord[1]= ( tc_y + tex_scale ) + ( tc_x << 4 );

            shred_vertices[2].coord[0]= x + X;
            shred_vertices[2].coord[1]= y - 1 + Y;
            shred_vertices[2].coord[2]= z;
            shred_vertices[2].tex_coord[0]= tex_id;
            shred_vertices[2].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 ); //1

            shred_vertices[ v13[0] ].coord[0]= x + X;
            shred_vertices[ v13[0] ].coord[1]= y - 1 + Y;
            shred_vertices[ v13[0] ].coord[2]= z - 1;
            shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
            shred_vertices[ v13[0] ].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 );//3

            shred_vertices[0].coord[0]= x + X;
            shred_vertices[0].coord[1]= y + Y;
            shred_vertices[0].coord[2]= z - 1;
            shred_vertices[0].tex_coord[0]= tex_id;
            shred_vertices[0].tex_coord[1]= tc_y | ( tc_x << 4 );//2

            shred_vertices[3].normal_id=
                shred_vertices[2].normal_id=
                    shred_vertices[1].normal_id=
                        shred_vertices[0].normal_id= normal_id;

            shred_vertices[3].light=
                shred_vertices[2].light=
                    shred_vertices[1].light=
                        shred_vertices[0].light= light;
            shred_vertices+= 4;
        }
        if( current != y1 )
        {
            normal_id= WORLD_NORMAL_Y;
            if( current > y1 )
            {
                v13[0]= 1;
                v13[1]= 3;
                normal_id++;
                b= south_shred->shred->GetBlock( x, 0, z );
                light= shred->LightMap( x, y, z );
                //light= south_shred->shred->LightMap( x, 0, z );
            }
            else
            {
                v13[0]= 3;
                v13[1]= 1;
                b= shred->GetBlock( x, y, z );
                light= south_shred->shred->LightMap( x, 0, z );
                //light= shred->LightMap( x, y, z );
            }

            tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
            tex_scale= rGetBlockTextureScale( tex_id );
            tc_x= ( x * tex_scale )&7;
            tc_y= ( z * tex_scale )&7;


            shred_vertices[0].coord[0]= x + X;
            shred_vertices[0].coord[1]= y + Y;
            shred_vertices[0].coord[2]= z;
            shred_vertices[0].tex_coord[0]= tex_id;
            shred_vertices[0].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 );//3


            shred_vertices[ v13[0] ].coord[0]= x - 1 + X;
            shred_vertices[ v13[0] ].coord[1]= y + Y;
            shred_vertices[ v13[0] ].coord[2]= z;
            shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
            shred_vertices[ v13[0] ].tex_coord[1]= (tc_y + tex_scale ) | ( tc_x << 4 );//2

            shred_vertices[2].coord[0]= x - 1 + X;
            shred_vertices[2].coord[1]= y + Y;
            shred_vertices[2].coord[2]= z - 1;
            shred_vertices[2].tex_coord[0]= tex_id ;
            shred_vertices[2].tex_coord[1]= tc_y | ( tc_x << 4 );//0

            shred_vertices[ v13[1] ].coord[0]= x + X;
            shred_vertices[ v13[1] ].coord[1]= y + Y;
            shred_vertices[ v13[1] ].coord[2]= z - 1;
            shred_vertices[ v13[1] ].tex_coord[0]= tex_id;
            shred_vertices[ v13[1] ].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 ); //1

            shred_vertices[3].normal_id=
                shred_vertices[2].normal_id=
                    shred_vertices[1].normal_id=
                        shred_vertices[0].normal_id= normal_id;

            shred_vertices[3].light=
                shred_vertices[2].light=
                    shred_vertices[1].light=
                        shred_vertices[0].light= light;
            shred_vertices+= 4;
        }
        if( current != z1 )
        {
            normal_id= WORLD_NORMAL_Z;
            if( current > z1 )
            {
                v13[0]= 1;
                v13[1]= 3;
                normal_id++;
                b= shred->GetBlock( x, y, z + 1 );
                light= shred->LightMap( x, y, z );
                //light= shred->LightMap( x, y, z + 1 );
            }
            else
            {
                v13[0]= 3;
                v13[1]= 1;
                b= shred->GetBlock( x, y, z );
                light= shred->LightMap( x, y,  z + 1 );
                //light= shred->LightMap( x, y, z );
            }


            tex_id= rGetBlockTexture( b->Sub(), b->Kind(), normal_id );
            tex_scale= rGetBlockTextureScale( tex_id );
            tc_x= ( x * tex_scale )&7;
            tc_y= ( -y * tex_scale )&7;

            shred_vertices[0].coord[0]= x + X;
            shred_vertices[0].coord[1]= y + Y;
            shred_vertices[0].coord[2]= z;
            shred_vertices[0].tex_coord[0]= tex_id;
            shred_vertices[0].tex_coord[1]= tc_y | ( ( tc_x + tex_scale ) << 4 ); //1

            shred_vertices[ v13[0] ].coord[0]= x + X;
            shred_vertices[ v13[0] ].coord[1]= y - 1 + Y;
            shred_vertices[ v13[0] ].coord[2]= z;
            shred_vertices[ v13[0] ].tex_coord[0]= tex_id;
            shred_vertices[ v13[0] ].tex_coord[1]= ( tc_y + tex_scale ) | ( ( tc_x + tex_scale ) << 4 );//3


            shred_vertices[2].coord[0]= x - 1 + X;
            shred_vertices[2].coord[1]= y - 1 + Y;
            shred_vertices[2].coord[2]= z;
            shred_vertices[2].tex_coord[0]= tex_id;
            shred_vertices[2].tex_coord[1]= ( tc_y + tex_scale ) | ( tc_x << 4 );//3

            shred_vertices[ v13[1] ].coord[0]= x - 1 + X;
            shred_vertices[ v13[1] ].coord[1]= y + Y;
            shred_vertices[ v13[1] ].coord[2]= z;
            shred_vertices[ v13[1] ].tex_coord[0]= tex_id ;
            shred_vertices[ v13[1] ].tex_coord[1]= tc_y | ( tc_x << 4 );//2

            shred_vertices[3].normal_id=
                shred_vertices[2].normal_id=
                    shred_vertices[1].normal_id=
                        shred_vertices[0].normal_id= normal_id;

            shred_vertices[3].light=
                shred_vertices[2].light=
                    shred_vertices[1].light=
                        shred_vertices[0].light= light;
            shred_vertices+= 4;
        }
    }
}

unsigned short r_ShredInfo::GetShredDistance( short x, short y )
{
    short dx, dy;
    dx= abs( x - latitude );
    dy= abs( y - longitude );
    return max( dx, dy );
}
bool r_ShredInfo::IsOnOtherSideOfPlane( m_Vec3 point, m_Vec3 normal )
{
    m_Vec3 vec2point;
    float dot;

    vec2point= m_Vec3( latitude * R_SHRED_WIDTH, longitude * R_SHRED_WIDTH, min_geometry_height ) - point;
    dot= vec2point * normal;
    if( dot > 0.0 )
        return true;

    vec2point= m_Vec3( ( latitude + 1 ) * R_SHRED_WIDTH, longitude * R_SHRED_WIDTH, min_geometry_height ) - point;
    dot= vec2point * normal;
    if( dot > 0.0 )
        return true;

    vec2point= m_Vec3( ( latitude + 1 ) * R_SHRED_WIDTH, ( longitude + 1 ) * R_SHRED_WIDTH, min_geometry_height ) - point;
    dot= vec2point * normal;
    if( dot > 0.0 )
        return true;

    vec2point= m_Vec3( latitude * R_SHRED_WIDTH, ( longitude + 1 ) * R_SHRED_WIDTH, min_geometry_height ) - point;
    dot= vec2point * normal;
    if( dot > 0.0 )
        return true;

    //top side
    vec2point= m_Vec3( latitude * R_SHRED_WIDTH, longitude * R_SHRED_WIDTH, max_geometry_height ) - point;
    dot= vec2point * normal;
    if( dot > 0.0 )
        return true;

    vec2point= m_Vec3( ( latitude + 1 ) * R_SHRED_WIDTH, longitude * R_SHRED_WIDTH, max_geometry_height + 1 ) - point;
    dot= vec2point * normal;
    if( dot > 0.0 )
        return true;

    vec2point= m_Vec3( ( latitude + 1 ) * R_SHRED_WIDTH, ( longitude + 1 ) * R_SHRED_WIDTH, max_geometry_height + 1 ) - point;
    dot= vec2point * normal;
    if( dot > 0.0 )
        return true;

    vec2point= m_Vec3( latitude * R_SHRED_WIDTH, ( longitude + 1 ) * R_SHRED_WIDTH, max_geometry_height + 1 ) - point;
    dot= vec2point * normal;
    if( dot > 0.0 )
        return true;

    return false;
}

#endif// SHRED_INFO_CPP
