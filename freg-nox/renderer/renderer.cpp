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

#ifndef RENDERER_CPP
#define RENDERER_CPP

#include "renderer.h"
#include "../screen.h"
#include "../Player.h"
r_Renderer* r_Renderer::current_renderer= NULL;

void r_Renderer::CalculateFPS()
{
    unsigned int new_time = -QTime::currentTime().msecsTo( startup_time );

    if( new_time - last_fps_time > 1000 )
    {
        fps= 1000 * (  frame_count - last_frame_number  ) / ( new_time - last_fps_time );
        last_fps_time= new_time;
        last_frame_number= frame_count;

        max_fps_to_draw= max_fps;
        min_fps_to_draw= min_fps;
        max_fps= 0.0;
        min_fps= 2000.0;
        shreds_update_per_second_to_draw= shreds_update_per_second;
        shreds_update_per_second= 0;
    }

    if( new_time - last_frame_time  > 0 )
    {
        max_fps= max( 1000.0 / float( new_time - last_frame_time ), max_fps );
        //max_fps= max( max_fps, float( new_time - last_frame_time ) );
        min_fps= min( 1000.0 / float( new_time - last_frame_time ), min_fps );
        //min_fps= min( min_fps, float( new_time - last_frame_time ) );
    }
    last_frame_time= new_time;
    frame_count ++;
}

void r_Renderer::SetupVertexBuffers()
{
    r_WorldVertex vert;

    temp_buffer.VertexData( NULL, 48, sizeof( r_WorldVertex ), 0 );
    temp_buffer.IndexData( NULL, 64, GL_UNSIGNED_INT, GL_TRIANGLES );

    int t= (int)(((char*)vert.coord) - ((char*)&vert));
    temp_buffer.VertexAttribPointer( ATTRIB_POSITION, 3, GL_SHORT, false, t  );
    t= (int)(((char*)vert.tex_coord) - ((char*)&vert));
    temp_buffer.VertexAttribPointer( ATTRIB_TEX_COORD, 2, GL_UNSIGNED_BYTE, false, t );
    t= (int)(((char*)&vert.normal_id) - ((char*)&vert));
    temp_buffer.VertexAttribPointer( ATTRIB_NORMAL, 1, GL_UNSIGNED_BYTE, false, t );
    t= (int)(((char*)&vert.light) - ((char*)&vert));
    temp_buffer.VertexAttribPointer( /*ATTRIB_USER0*/3/*light*/ , 1, GL_UNSIGNED_BYTE, false, t );



    world_buffer.VertexData( (float*)vertex_buffer, sizeof(r_WorldVertex) * vertex_buffer_size ,sizeof(r_WorldVertex) , 0 );
    world_buffer.IndexData( indeces, sizeof(quint32) * index_buffer_size, GL_UNSIGNED_INT, GL_TRIANGLES );


    t= (int)(((char*)vert.coord) - ((char*)&vert));
    world_buffer.VertexAttribPointer( ATTRIB_POSITION, 3, GL_SHORT, false, t  );
    t= (int)(((char*)vert.tex_coord) - ((char*)&vert));
    world_buffer.VertexAttribPointer( ATTRIB_TEX_COORD, 2, GL_UNSIGNED_BYTE, false, t );
    t= (int)(((char*)&vert.normal_id) - ((char*)&vert));
    world_buffer.VertexAttribPointer( ATTRIB_NORMAL, 1, GL_UNSIGNED_BYTE, false, t );
    t= (int)(((char*)&vert.light) - ((char*)&vert));
    world_buffer.VertexAttribPointer( /*ATTRIB_USER0*/3/*light*/ , 1, GL_UNSIGNED_BYTE, false, t );


    float sky_vertices[]= { 256.0, 256.0, 128.0, -256.0, 256.0, 128.0,
                            256.0, -256.0, 128.0, -256.0, -256.0, 128.0,
                            256.0, 256.0, -128.0, -256.0, 256.0, -128.0,
                            256.0, -256.0, -128.0, -256.0, -256.0, -128.0
                          };
    quint16 sky_indeces[]= { 0, 1, 5,  0, 5, 4,
                             0, 4, 6,  0, 6, 2,
                             4, 5, 7,  4, 7, 6,
                             0, 3, 1,  0, 2, 3, //top
                             2, 7, 3,  2, 6, 7,
                             1, 3, 7,  1, 7, 5
                           };
    sky_buffer.VertexData( sky_vertices, sizeof(float) * 8 * 3, sizeof(float) * 3, 0 );
    sky_buffer.IndexData( (quint32*)sky_indeces, sizeof(quint16) * 36, GL_UNSIGNED_SHORT, GL_TRIANGLES );
    sky_buffer.VertexAttribPointer( 0, 3, GL_FLOAT, false, 0 );


    text_buffer.VertexData( NULL, R_LETTER_BUFFER_LEN * 4 * sizeof(r_FontVertex),
                            sizeof(r_FontVertex), 0 );
    text_buffer.IndexData( (quint32*) font_indeces,
                           R_LETTER_BUFFER_LEN * 6 * sizeof(quint16),
                           GL_UNSIGNED_SHORT, GL_TRIANGLES );
    r_FontVertex fv;
    t= (int)(((char*)fv.coord) - ((char*)&fv));
    text_buffer.VertexAttribPointer( ATTRIB_POSITION,   2, GL_FLOAT, false, t );
    t= (int)(((char*)fv.tex_coord) - ((char*)&fv));
    text_buffer.VertexAttribPointer( ATTRIB_TEX_COORD,  2, GL_UNSIGNED_SHORT, false, t );
    t= (int)(((char*)fv.color) - ((char*)&fv));
    text_buffer.VertexAttribPointer( /*color*/2,        3, GL_UNSIGNED_BYTE, true , t );
    t= (int)(((char*)&fv.tex_id) - ((char*)&fv));
    text_buffer.VertexAttribPointer( /*tex_id*/3,       1, GL_UNSIGNED_BYTE, false , t );


    float fq[]= { 	1.0, 1.0,  1.0, -1.0,
                    -1.0, -1.0, -1.0, 1.0
                };
    quint16 fq_ind[]= { 0, 1, 2, 0, 2, 3 };
    fullscreen_quad.VertexData( fq, sizeof( float ) * 8, sizeof( float ) * 2,  0 );
    fullscreen_quad.IndexData( (unsigned int*) fq_ind, sizeof( quint16) * 6, GL_UNSIGNED_SHORT, GL_TRIANGLES );
    fullscreen_quad.VertexAttribPointer( 0, 2, GL_FLOAT, false, 0 );
}

void r_Renderer::DrawWorld()
{
    m_Vec3 frame_cam_pos= cam_position;
    m_Vec3 frame_cam_ang= cam_angle;
    m_Mat4 rotX, rotZ, per, tr, chang;
    m_Vec3 tr_vec= - frame_cam_pos;
    rotX.Identity();
    rotX.RotateX( frame_cam_ang.x - m_Math::FM_PI );
    rotZ.Identity();
    rotZ.RotateZ( -frame_cam_ang.z );
    tr.Identity();
    tr.Translate( tr_vec );
    chang.Identity();
    chang[10]= chang[5]= 0.0;
    chang[6]= 1.0;
    chang[9]= -1.0;

    per.MakePerspective( (float) viewport_x/ (float) viewport_y, fov, z_near, z_far );
    view_matrix= tr * rotZ * rotX *chang* per;



    m_Mat4 depth_tex_mat, result_depth_tex_mat;

    depth_tex_mat.Identity();

    /*depth_tex_mat[0]=
        depth_tex_mat[5]=
            depth_tex_mat[10]= 0.5f;

    depth_tex_mat[12]=
        depth_tex_mat[13]=
            depth_tex_mat[14]= 0.5f;*/


    result_depth_tex_mat= shadow_matrix * depth_tex_mat;


    texture_array.Bind(0);
    sun_shadow_map.BindDepthTexture(1);

    world_shader.Bind();
    world_shader.Uniform( "sun_vector", sun_vector );
    world_shader.Uniform( "tex", 0 );
    world_shader.Uniform( "shadow_map", 1 );
    world_shader.Uniform( "shadow_mat", result_depth_tex_mat );
    world_shader.Uniform( "proj_mat", view_matrix );
    world_shader.Uniform( "cam_pos", frame_cam_pos );
    if( world->PartOfDay() == NIGHT )
        world_shader.Uniform( "fog_color",(m_Vec3&)night_fog );
    else
        world_shader.Uniform( "fog_color",(m_Vec3&)day_fog );


    float max_view= 1.0 / ( float( world->NumShreds() - 1 ) * 0.5f * 16.0f );
    world_shader.Uniform( "max_view2", max_view * max_view );

    world_buffer.Bind();
    /*   for( unsigned int i= 0; i< shreds_to_draw_count; i++ )
       {

           glDrawElements( GL_TRIANGLES,
                           shreds_to_draw_list[i]->quad_count * 6,
                           GL_UNSIGNED_INT,
                           (void*)(shreds_to_draw_list[i]->index_buffer_offset * 6 * 4) );
       }*/
    glMultiDrawElements( GL_TRIANGLES, shreds_to_draw_quad_count,
                         GL_UNSIGNED_INT, (GLvoid* const*)shreds_to_draw_indeces,
                         shreds_to_draw_count );

}

void r_Renderer::DrawSky()
{
    sky_shader.Bind();
    sky_shader.Uniform( "tex", 0 );
    sky_shader.Uniform( "proj_mat", view_matrix );
    sky_shader.Uniform( "cam_pos", cam_position );
    sky_buffer.Show();
}

void r_Renderer::DrawHUD()
{
    m_Vec3 p;
    //технические данные
    p.y= 0.9;
    m_Vec3 text_color( 1.0, 1.0, 0.3 );
    p= AddText( -0.9, p.y, &text_color, 1, "fps: %d\n", fps );
    p= AddText( -0.9, p.y, &text_color, 1, "max fps: %2.1f\n", max_fps_to_draw );
    p= AddText( -0.9, p.y, &text_color, 1, "min fps: %2.1f\n", min_fps_to_draw );
    p= AddText( -0.9, p.y, &text_color, 1, "time: %dh %dm\n",
                ( world->Time() / seconds_in_hour ) % 24,
                world->Time() % seconds_in_hour );

    p= AddText( -0.9, p.y, &text_color, 1, "shreds: %d/%d\n", shreds_to_draw_count, visibly_world_size[0] * visibly_world_size[1] );
    p= AddText( -0.9, p.y, &text_color, 1, "shreds updated: %d\n", shreds_update_per_second_to_draw );

    //данные о здоровье
    text_color.x= text_color.y= text_color.z= 0.8;
    char hp_string[32];
    sprintf( hp_string, "HP: %3d\%[", player->HP() );

    p= AddText( 0.5, -0.7, &text_color, 1, hp_string );
    text_color.x= 0.8, text_color.y= 0.1, text_color.z= 0.1;
    for( int i=0; i< 10; i++ )
        p= AddText( p.x, p.y, &text_color, 1, ( player->HP()/10 <= i ) ? " " : "#"  );

    text_color.x= text_color.y= text_color.z= 0.8;
    p= AddText( p.x, p.y, &text_color, 1, "]\n" );

    //данные о дыхании
    sprintf( hp_string, "BR: %3d\%[", player->Breath() );
    p= AddText( 0.5, p.y, &text_color, 1, hp_string );
    text_color.x= 0.1, text_color.y= 0.1, text_color.z= 0.8;
    for( int i=0; i< 10; i++ )
        p= AddText( p.x, p.y, &text_color, 1, ( player->Breath()/10 <= i ) ? " " : "#"  );

    text_color.x= text_color.y= text_color.z= 0.8;
    p= AddText( p.x, p.y, &text_color, 1, "]\n" );


    //состояние голода
    text_color.x= 0.1, text_color.y= 0.8, text_color.z= 0.1;
    char* satiation_str;

    if ( seconds_in_day*time_steps_in_sec< player->Satiation() )
        satiation_str= "Gorged\n\n";

    else if ( 3*seconds_in_day*time_steps_in_sec/4 < player->Satiation() )
        satiation_str= "Full\n\n";

    else if (seconds_in_day*time_steps_in_sec/4> player->Satiation() )
        satiation_str= "Hungry\n\n";

    AddText( 0.5, p.y, &text_color, 1, satiation_str );

	text_color.x= text_color.y= text_color.z= 0.8;
	p.y= -0.6;
    for( int i= R_NUMBER_OF_NOTIFY_LINES - 1; i>= 0; i-- )
    {
    	if(  notify_log[ ( notify_log_first_line - 1 - i )%R_NUMBER_OF_NOTIFY_LINES][0] != 0 )
			 p=AddText( -0.99, p.y, &text_color, 1, "%s\n", notify_log[ ( notify_log_first_line - 1 - i )%R_NUMBER_OF_NOTIFY_LINES] );
    }
}

void r_Renderer::Draw()
{

    if( frame_count == 0 )
        BuildWorld();

    gpu_data_mutex.lock();

    if( frame_count == 0 )
        SetupVertexBuffers();

#if ! FREG_GL_MULTI_THREAD
    if( need_update_vertex_buffer )
    {
        host_data_mutex.lock();
        if( out_of_vertex_buffer )
        {
            world_buffer.VertexData( (float*)vertex_buffer, sizeof(r_WorldVertex) * vertex_buffer_size ,sizeof(r_WorldVertex) , 0 );
            if( out_of_index_buffer )
            {
                world_buffer.IndexData( indeces, sizeof(quint32) * index_buffer_size, GL_UNSIGNED_INT, GL_TRIANGLES );
                out_of_index_buffer= false;
            }
            out_of_vertex_buffer= false;
            full_update= false;

            memcpy( draw_shreds, shreds
                    , sizeof(r_ShredInfo) * visibly_world_size[0] * visibly_world_size[1] );
            need_full_update_shred_list= false;
        }
        else
        {
            r_ShredInfo* shred;
            for( unsigned int x= 0; x< visibly_world_size[0]; x++ )
            {
                for( unsigned int y= 0; y< visibly_world_size[1]; y++ )
                {
                    shred= &shreds[ x + y * visibly_world_size[0] ];
                    if( shred->rebuilded )
                    {
                        world_buffer.VertexSubData( (float*)shred->vertex_buffer,
                                                    shred->quad_count * 4 * sizeof( r_WorldVertex ),
                                                    (shred->vertex_buffer - vertex_buffer) * sizeof(r_WorldVertex) );

                        memcpy( &draw_shreds[ x + y * visibly_world_size[0] ], shred, sizeof( r_ShredInfo ) );
                        shred->rebuilded= false;
                        shreds_update_per_second++;
                    }
                }
            }
        }
        if( need_full_update_shred_list )
        {
            memcpy( draw_shreds, shreds,
                    sizeof(r_ShredInfo) * visibly_world_size[0] * visibly_world_size[1] );
            need_full_update_shred_list= false;
        }
        need_update_vertex_buffer= false;
        host_data_mutex.unlock();

    }
#else
#endif//FREG_GL_MULTI_THREAD





#if FREG_GL_MULTI_THREAD
    if( need_update_vertex_buffer )
    {
        unsigned char tmp[ sizeof( r_PolygonBuffer ) ];
        if( transmition_frame - frame_count > 60 )
        {
            glWaitSync( sync_object, 0, GL_TIMEOUT_IGNORED );
            need_update_vertex_buffer= false;
            printf( "sync" );
        }
    }
#endif

    //glCullFace( GL_BACK );
    RenderShadows();
    //glCullFace( GL_FRONT );

    glClear(GL_DEPTH_BUFFER_BIT );
    BuildShredList();
    // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); //WireFrame Mode
    DrawWorld();
    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ); //WireFrame Mode
    //shadowmap_transform_shader.Bind();
    //fullscreen_quad.Show();
    DrawSky();

    DrawHUD();

    DrawText();

    gpu_data_mutex.unlock();
    CalculateFPS();
}



void r_Renderer::RenderShadows()
{
    const float sun_max_horison_angle= m_Math::FM_TORAD * 60.0;
    m_Vec3 sun_pos;
    m_Mat4 translate, result, projection, rotateX,rotateY, rotateZ,rotateZ2, viewport, yzchange, sun_vec_rot_X, sun_vec_rot_Z;

    //расчёт угла солнца относительго времени дня
    float angle= m_Math::FM_PI * float( ( world->Time() % seconds_in_day / 60 ) * 60 - end_of_night )/float ( seconds_in_day - end_of_night );
    if( angle < MIN_SUN_HEIGHT )  angle= 0.17;
    else if ( angle > ( m_Math::FM_PI - MIN_SUN_HEIGHT ) ) angle= 2.96;

    //расчёт позиции центра карты теней
    Shred* player_shred= world->GetShred( player->X(), player->Y() );
     sun_pos.x= - float( player_shred->Latitude() ) * float( R_SHRED_WIDTH ) + float( R_SHRED_WIDTH/2 );
     sun_pos.y= - float( player_shred->Longitude() ) * float( R_SHRED_WIDTH ) + float( R_SHRED_WIDTH/2 );
     sun_pos.z= - float(R_SHRED_HEIGHT/2 + R_SHRED_HEIGHT/4);
    //sun_pos= -cam_position;

    //расчёт вектора солнца
    sun_vector= m_Vec3( 0.0, 1.0, 0.0 );
    sun_vec_rot_Z.Identity(), sun_vec_rot_Z.RotateZ( angle - m_Math::FM_PI2 );
    sun_vec_rot_X.Identity(), sun_vec_rot_X.RotateX( sun_max_horison_angle );
    sun_vec_rot_Z= sun_vec_rot_Z * sun_vec_rot_X;
    sun_vector= sun_vector * sun_vec_rot_Z;
    if( world->PartOfDay() == NIGHT )
        sun_vector= m_Vec3( 0.0, 0.0, 0.0 );



    //расчёт матрицы для рендеринга теней
    translate.Identity();
    translate.Translate( sun_pos );
    rotateX.Identity();
    rotateX.RotateX( m_Math::FM_PI - angle );
    rotateZ.Identity();
    rotateZ.RotateZ( m_Math::FM_PI2 );
    rotateY.Identity();
    rotateY.RotateY( m_Math::FM_PI2 - sun_max_horison_angle );

    yzchange.Identity();
    yzchange[10]= yzchange[5]= 0.0;
    yzchange[6]= 1.0;
    yzchange[9]= -1.0;

    //расчёт матнрицы проекции
    projection.Identity();
    float l= visibly_world_size[0] * visibly_world_size[1] * 2.0 *
             float( R_SHRED_WIDTH * R_SHRED_WIDTH ) + float( R_SHRED_HEIGHT/2 * R_SHRED_HEIGHT/2 );
    l= sqrt(l) + 16.0;//диаганаль параллелепипеда мира
    projection.MakeProjection( 2.0 / l, 2.0 / l, -0.5 * l, 0.5 * l );

    result= translate * rotateZ * rotateY * rotateX * yzchange * projection;
    shadow_matrix= result;

    sun_shadow_map.Bind();
    sun_shadow_map.ClearBuffer( true, true );
    shadow_shader.Bind();
    shadow_shader.Uniform( "proj_mat", result );

    world_buffer.Bind();
    r_ShredInfo* shred;
    for( unsigned int i= 0; i< visibly_world_size[0]; i++ )
    {
        for( unsigned int j=0; j< visibly_world_size[1]; j++ )
        {
            shred= &draw_shreds[ i + j * visibly_world_size[0] ];

            shreds_to_draw_quad_count[ i + j * visibly_world_size[0] ] = shred->quad_count * 6;
            shreds_to_draw_indeces[ i + j * visibly_world_size[0] ]= shred->index_buffer_offset * 6 * sizeof( quint32);
            /* glDrawElements( GL_TRIANGLES,
                             shred->quad_count * 6,
                             GL_UNSIGNED_INT,
                             (void*)(shred->index_buffer_offset * 6 * 4) );*/
        }

    }
    shreds_to_draw_count= visibly_world_size[0] * visibly_world_size[1];
    glMultiDrawElements( GL_TRIANGLES, shreds_to_draw_quad_count,
                         GL_UNSIGNED_INT, (GLvoid* const*)shreds_to_draw_indeces,
                         shreds_to_draw_count );


    //замена значений 1 в текстуре глубины на 0
	/*glDepthFunc( GL_ALWAYS );//отключение теста глубины, чтобы не чистить буффер
    sun_shadow_map.Bind();
    shadowmap_transform_shader.Bind();
    raw_sun_shadow_map.BindDepthTexture( 1 );
	shadowmap_transform_shader.Uniform( "shadowmap", 1 );
    fullscreen_quad.Show();*/
    sun_shadow_map.BindNull();

   // glDepthFunc( GL_LEQUAL );//включение теста глубины обратно

    glViewport( 0, 0, viewport_x, viewport_y );
}
void r_Renderer::BuildWorld()
{
    host_data_mutex.lock();
    unsigned int x, y;
    Shred* shred;
    r_ShredInfo* cur;

    /*определение количества квадов*/
    vertices_in_buffer= 0;
    for( x= 0; x< visibly_world_size[0]; x++ )
    {
        for( y= 0; y< visibly_world_size[1]; y++ )
        {
            shred= (Shred*) world->GetShred( x * 16, y * 16 );
            cur= &shreds[ y * visibly_world_size[0] + x ];

            cur->visibly_information= visibly_information + 16 * 16 * 128 * ( y * visibly_world_size[0] + x );
            cur->latitude= shred->Latitude();//x;
            cur->longitude= shred->Longitude();//y;
            cur->shred = shred;

            if( x < ( visibly_world_size[0] - 1 ) )
                cur->east_shred= &shreds[ y * visibly_world_size[0] + x + 1 ];
            else
                cur->east_shred= NULL;

            if( y < ( visibly_world_size[1] - 1 ) )
                cur->south_shred= &shreds[ (y + 1)* visibly_world_size[0] + x ];
            else
                cur->south_shred= NULL;

            if( x >= 1 )
                cur->west_shred= &shreds[ y * visibly_world_size[0] + x - 1 ];
            else
                cur->west_shred= NULL;

            if( y >= 1 )
                cur->north_shred= &shreds[ (y - 1)* visibly_world_size[0] + x ];
            else
                cur->north_shred= NULL;

            cur->GetVisiblyInformation();
            cur->visibly_information_is_valid= true;
        }
    }

    /*построение геометрии*/
    for( x= 0; x< visibly_world_size[0]; x++ )
    {
        for( y= 0; y< visibly_world_size[1]; y++ )
        {
            cur= &shreds[ y * visibly_world_size[0] + x ];

            cur->GetQuadCount();
            cur->updated= false;
            cur->rebuilded= false;
            cur->need_update_quads= false;

            cur->quad_buffer_count= cur->quad_count + ( cur->quad_count>>2 );
            vertices_in_buffer+= cur->quad_count * 4;
            vertex_buffer_size+= cur->quad_buffer_count * 4;
        }
    }
    vertex_buffer= new  r_WorldVertex[ vertex_buffer_size ];
    r_WorldVertex* v= vertex_buffer;
    unsigned int i_offset= 0;

    // построение геометрии
    for( x= 0; x< visibly_world_size[0]; x++ )
    {
        for( y= 0; y< visibly_world_size[1]; y++ )
        {
            cur= &shreds[ y * visibly_world_size[0] + x ];

            cur->index_buffer_offset= i_offset;
            cur->vertex_buffer= v;

            cur->BuildShred( v );

            v+= cur->quad_buffer_count * 4;
            i_offset+= cur->quad_buffer_count;

        }
    }
    //заполнение индексного буффера
    index_buffer_size= 6 * vertex_buffer_size / 4;
    indeces= new quint32[ index_buffer_size ];

    for( x= 0, y=0; x< index_buffer_size; x+=6, y+=4 )
    {
        indeces[x] = y;
        indeces[x + 1] = y + 1;
        indeces[x + 2] = y + 2;

        indeces[x + 3] = y;
        indeces[x + 4] = y + 2;
        indeces[x + 5] = y + 3;
    }

    shred= world->GetShred( 16 * visibly_world_size[0]/2, 16 * visibly_world_size[1]/2 );
    center_shred_latitude= shred->Latitude();
    center_shred_longitude= shred->Longitude();

    memcpy( draw_shreds, shreds
            , sizeof(r_ShredInfo) * visibly_world_size[0] * visibly_world_size[1] );

    renderer_initialized= true;
    need_update_vertex_buffer= false;
    host_data_mutex.unlock();
    world->Unlock();

}

void r_Renderer::UpdateData()
{
    host_data_mutex.lock();
    r_ShredInfo* shred;
    Shred* world_shred;
    unsigned int x, y;
    unsigned int new_quad_count= 0, new_index_buffer_size;
    //unsigned int* new_index_buffer;
    r_WorldVertex* new_vertex_buffer, *v;
    unsigned int i_offset;
    unsigned char tmp[ sizeof( r_PolygonBuffer ) ];
    bool full_shred_list_update= false;
    if( full_update )
        goto out_point;


    for( x= 0; x< visibly_world_size[0]; x++ )
    {
        for( y= 0; y< visibly_world_size[1]; y++ )
        {
            shred= &shreds[ x + y * visibly_world_size[0] ];
            if( shred->updated )
            {
                shred->GetQuadCount();
                shred->need_update_quads = false;
                if( shred->quad_buffer_count < shred->quad_count )
                {
                    out_of_vertex_buffer= true;
                    goto out_point;
                }
            }
        }
    }

out_point:
    /*если буффер надо пересоздать*/
    if( full_update || out_of_vertex_buffer )
    {
        printf( "out of vertex buffer\n" );
        if( full_update )
        {
            for( x= 0; x< visibly_world_size[0]; x++ )
            {
                for( y= 0; y< visibly_world_size[1]; y++ )
                {

                    world_shred= (Shred*) world->GetShred( x * 16, y * 16 );
                    shred= &shreds[ x + y * visibly_world_size[0] ];

                    shred->latitude= world_shred->Latitude();//x;
                    shred->longitude= world_shred->Longitude();//y;
                    shred->shred = world_shred;


                    if( x < ( visibly_world_size[0] - 1 ) )
                        shred->east_shred= &shreds[ y * visibly_world_size[0] + x + 1 ];
                    else
                        shred->east_shred= NULL;

                    if( y < ( visibly_world_size[1] - 1 ) )
                        shred->south_shred= &shreds[ (y + 1)* visibly_world_size[0] + x ];
                    else
                        shred->south_shred= NULL;

                    if( x >= 1 )
                        shred->west_shred= &shreds[ y * visibly_world_size[0] + x - 1 ];
                    else
                        shred->west_shred= NULL;

                    if( y >= 1 )
                        shred->north_shred= &shreds[ (y - 1)* visibly_world_size[0] + x ];
                    else
                        shred->north_shred= NULL;

                    shred->visibly_information= visibly_information + 16 * 16 * 128 * ( y * visibly_world_size[0] + x );
                    shred->GetVisiblyInformation();
                    shred->visibly_information_is_valid= true;
                }
            }
        }
        for( x= 0; x< visibly_world_size[0]; x++ )
        {
            for( y= 0; y< visibly_world_size[1]; y++ )
            {
                shred= &shreds[ x + y * visibly_world_size[0] ];
                shred->GetQuadCount();
                shred->quad_buffer_count= shred->quad_count + (shred->quad_count>>2);
                new_quad_count+= shred->quad_buffer_count;
            }
        }

        new_index_buffer_size= new_quad_count * 6;
        new_vertex_buffer= new r_WorldVertex[ new_quad_count * 4 ];
        v= new_vertex_buffer;
        i_offset= 0;
        for( x= 0; x< visibly_world_size[0]; x++ )
        {
            for( y= 0; y< visibly_world_size[1]; y++ )
            {
                shred= &shreds[ x + y * visibly_world_size[0] ];

                shred->BuildShred( v );

                shred->vertex_buffer= v;
                shred->index_buffer_offset= i_offset;
                v+= shred->quad_buffer_count * 4;
                i_offset+= shred->quad_buffer_count;
            }
        }

        if( new_index_buffer_size > index_buffer_size )
        {
            delete[] indeces;
            index_buffer_size= new_index_buffer_size;
            indeces= new quint32[ index_buffer_size ];
            out_of_index_buffer= true;
            for( x= 0, y=0; x< index_buffer_size; x+=6, y+=4 )
            {
                indeces[x] = y;
                indeces[x + 1] = y + 1;
                indeces[x + 2] = y + 2;

                indeces[x + 3] = y;
                indeces[x + 4] = y + 2;
                indeces[x + 5] = y + 3;
            }
        }
        delete[] vertex_buffer;
        vertex_buffer= new_vertex_buffer;
        vertex_buffer_size= new_quad_count * 4;

        need_update_vertex_buffer= true;
        full_update= true;
        out_of_vertex_buffer= true;
    }

    else// если только надо обновить буффер
    {
        for( x= 0; x< visibly_world_size[0]; x++ )
        {
            for( y= 0; y< visibly_world_size[1]; y++ )
            {
                shred= &shreds[ x + y * visibly_world_size[0] ];
                if( shred->updated )
                {
                    bool need_rebuild = shred->immediately_update;
                    if( shred->immediately_update )
                        full_shred_list_update= true;
                    shred->immediately_update= false;

                    unsigned short shred_distance= shred->GetShredDistance( center_shred_latitude, center_shred_longitude );
                    if( shred_distance < r_Config.double_update_interval_radius )
                        need_rebuild= true;

                    else if( shred_distance < r_Config.quad_update_intrval_radius )
                    {
                        if( ( ( shred->latitude + shred->latitude + update_count ) & 1 ) == 0 )
                            need_rebuild= true;
                    }
                    else if( shred_distance < r_Config.octal_update_interval_radius )
                    {
                        if( ( ( shred->latitude + shred->latitude + update_count ) & 3 ) == 0 )
                            need_rebuild= true;
                    }
                    else
                    {
                        if( ( ( shred->latitude + shred->latitude + update_count ) & 7 ) == 0 )
                            need_rebuild= true;
                    }
                    if( need_rebuild )
                    {
                        shred->BuildShred( shred->vertex_buffer );
                        shred->rebuilded= true;
                        shred->updated= false;
                        need_update_vertex_buffer= true;
                    }

                }
            }
        }
    }
    if( full_shred_list_update )
        need_full_update_shred_list= true;
    host_data_mutex.unlock();
}



m_Vec3 r_Renderer::AddText( float x, float y, const m_Vec3* color, unsigned int size,  const char* text, ... )
{
    if( letter_count >= R_LETTER_BUFFER_LEN )
        return m_Vec3( x, y, 0.0 );

    char str[ 2048 ];
    va_list ap;
    va_start( ap, text );
    vsprintf( str, text, ap );
    va_end( ap );//дописываем в строку последние неизавестные параметры

    unsigned int len= strlen( str );

    if( len + letter_count >= R_LETTER_BUFFER_LEN )
        len-= ( R_LETTER_BUFFER_LEN - letter_count -1 );
    unsigned int no_type_symbols= 0;
    unsigned int i, j;
    unsigned short l;
    float pos_x= x, pos_y= y, h, dx;

    unsigned char c_color[3];
    unsigned short tex_id;
    c_color[0]= (unsigned char)(color->x * 255.0f);
    c_color[1]= (unsigned char)(color->y * 255.0f);
    c_color[2]= (unsigned char)(color->z * 255.0f);

    h= float( size * font.LetterHeight() ) /  float( viewport_y>>1 );//высота буквы в экранных координатах
    dx= h * float(viewport_y) / float(viewport_x);//этотт же размер, но по оси y
    for( i= 0, j= letter_count * 4; i< len; i++, j+=4 )
    {
        l= (unsigned short) str[i];
        if( l == '\n' )
        {
            pos_x= x;
            no_type_symbols++;
            pos_y-= h;
            j-=4;
            continue;
        }
        tex_id= font.TextureNum( l );

        font_vertices[j].coord[0]= pos_x;
        font_vertices[j].coord[1]= pos_y;
        font_vertices[j].tex_coord[0]= font.TexCoordLeft( l );
        font_vertices[j].tex_coord[1]= font.TexCoordBottom( l );

        font_vertices[j+1].coord[0]= pos_x;
        font_vertices[j+1].coord[1]= pos_y + h;
        font_vertices[j+1].tex_coord[0]= font.TexCoordLeft( l );
        font_vertices[j+1].tex_coord[1]= font.TexCoordTop( l );

        font_vertices[j+2].coord[0]= pos_x + font.LetterWidth( l ) * dx;
        font_vertices[j+2].coord[1]= pos_y + h;
        font_vertices[j+2].tex_coord[0]= font.TexCoordRight( l );
        font_vertices[j+2].tex_coord[1]= font.TexCoordTop( l );

        font_vertices[j+3].coord[0]= pos_x + font.LetterWidth( l ) * dx;
        font_vertices[j+3].coord[1]= pos_y;
        font_vertices[j+3].tex_coord[0]= font.TexCoordRight( l );
        font_vertices[j+3].tex_coord[1]= font.TexCoordBottom( l );

        font_vertices[j].tex_id=
            font_vertices[j+1].tex_id=
                font_vertices[j+2].tex_id=
                    font_vertices[j+3].tex_id= tex_id;

        font_vertices[j].color[0]=
            font_vertices[j+1].color[0]=
                font_vertices[j+2].color[0]=
                    font_vertices[j+3].color[0]= c_color[0];

        font_vertices[j].color[1]=
            font_vertices[j+1].color[1]=
                font_vertices[j+2].color[1]=
                    font_vertices[j+3].color[1]= c_color[1];

        font_vertices[j].color[2]=
            font_vertices[j+1].color[2]=
                font_vertices[j+2].color[2]=
                    font_vertices[j+3].color[2]= c_color[2];

        pos_x+= font.LetterWidth( l ) * dx;

    }
    letter_count+= len - no_type_symbols;

    return m_Vec3( pos_x, pos_y, 0.0 );
}

void r_Renderer::DrawText()
{
    text_buffer.VertexSubData( (float*) font_vertices,
                               letter_count * 4 * sizeof( r_FontVertex ), 0 );


    font.FontTexture( R_FONT_PAGE_ASCII )->BindTexture();
    text_shader.Bind();
    text_shader.Uniform( "tex", 0 );
    text_buffer.Bind();
    glDrawElements( GL_TRIANGLES, letter_count * 6, GL_UNSIGNED_SHORT, 0 );
    letter_count= 0;

}

void r_Renderer::StartUpText()
{
    text_shader.Load( "shaders/text_frag.glsl", "shaders/text_vert.glsl" );
    text_shader.MoveOnGPU();

    text_shader.FindUniform( "tex" );
    text_shader.FindAttrib( "coord" );
    text_shader.FindAttrib( "tex_coord" );
    text_shader.FindAttrib( "color" );

    text_shader.SetAttribLocation( "coord", ATTRIB_POSITION );
    text_shader.SetAttribLocation( "tex_coord", ATTRIB_TEX_COORD );
    text_shader.SetAttribLocation( "color", 2 );
    text_shader.SetAttribLocation( "tex_id", 3 );

    font.LoadFontPage( R_FONT_PAGE_ASCII, "fonts/Courier_New_12.txt", "fonts/cn.bmp" );


    font_vertices= new r_FontVertex[ R_LETTER_BUFFER_LEN * 4 ];
    font_indeces= new quint16[ R_LETTER_BUFFER_LEN * 6 ];


    for( unsigned short i=0, j=0; j< R_LETTER_BUFFER_LEN * 6; i+=4, j+=6 )
    {
        font_indeces[j  ]= i    ;
        font_indeces[j+1]= i + 1;
        font_indeces[j+2]= i + 2;
        font_indeces[j+3]= i    ;
        font_indeces[j+4]= i + 2;
        font_indeces[j+5]= i + 3;
    }
    letter_count= 0;
}
void r_Renderer::UpdateTick()
{
    int t0= -QTime::currentTime().msecsTo( startup_time ), dt;
    if( renderer_initialized )
    {
        world->ReadLock();
        UpdateData();
        world->Unlock();
        update_count++;
    }
    dt= -QTime::currentTime().msecsTo( startup_time ) - t0;

    if( dt > r_Config.update_interval )
        return;

    usleep( ( r_Config.update_interval - dt ) * 1000 );
}

void r_Renderer::LoadTextures()
{
    r_TextureFile tf;
    if( rLoadTextureBMP( &tf, "textures/texture_array.bmp" ) )
    {
        rDefaultTexture( &tf );
        printf( "error, world texture atlas not found\n" );
    }
    unsigned char ck[]= { 0, 120, 0 };
    rRGB2RGBAKeyColor( &tf, ck );

    texture_array.TextureData( tf.width, tf.width, tf.height/tf.width, GL_UNSIGNED_BYTE, GL_RGBA, 32, tf.data );
    texture_array.MoveOnGPU();

    /* rLoadTextureBMP( &tf, "textures/grass.bmp" );
      texture_array.TextureLayer( 9, tf.data );
      texture_array.TextureLayer( 10, tf.data );
      texture_array.TextureLayer( 13, tf.data );
      texture_array.TextureLayer( 15, tf.data );
      delete[] tf.data;

      rLoadTextureBMP( &tf, "textures/null_stone.bmp" );
      texture_array.TextureLayer( 0, tf.data );
      texture_array.TextureLayer( 1, tf.data );
      texture_array.TextureLayer( 2, tf.data );
      delete[] tf.data;

      rLoadTextureBMP( &tf, "textures/dirt_grass.bmp" );
      texture_array.TextureLayer( 6, tf.data );
      delete[] tf.data;

      rLoadTextureBMP( &tf, "textures/player.bmp" );
      texture_array.TextureLayer( 7, tf.data );
      delete[] tf.data;

      rLoadTextureBMP( &tf, "textures/wood.bmp" );
      texture_array.TextureLayer( 11, tf.data );
      delete[] tf.data;

      texture_array.GenerateMipmap();*/
    texture_array.DeleteFromRAM();
    texture_array.SetFiltration( GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
}
void r_Renderer::Initialize()
{
    gpu_data_mutex.lock();


    GetGLFunctions();
    printf( "functions ready\n" );
    current_renderer= this;

    if( world_shader.Load( "shaders/world_frag.glsl",
                           "shaders/world_vert.glsl", NULL ) )
        printf( "error, world shader not found\n" );

    world_shader.MoveOnGPU();
    world_shader.FindUniform( "tex" );
    world_shader.FindUniform( "shadow_map" );
    world_shader.FindUniform( "shadow_mat" );
    world_shader.FindUniform( "proj_mat" );
    world_shader.FindUniform( "normal_mat" );
    world_shader.FindUniform( "cam_pos" );
    world_shader.FindUniform( "fog_color" );
    world_shader.FindUniform( "max_view2" );
    world_shader.FindUniform( "sun_vector" );

    world_shader.FindAttrib( "coord" );
    world_shader.FindAttrib( "tex_coord" );
    world_shader.FindAttrib( "normal" );
    world_shader.FindAttrib( "light" );

    world_shader.SetAttribLocation( "coord", ATTRIB_POSITION );
    world_shader.SetAttribLocation( "tex_coord", ATTRIB_TEX_COORD );
    world_shader.SetAttribLocation( "normal", ATTRIB_NORMAL );
    world_shader.SetAttribLocation( "light", /*ATTRIB_USER0*/3 );


    if( sky_shader.Load( "shaders/sky_frag.glsl", "shaders/sky_vert.glsl", NULL ) )
        printf( "error, sky shader not found\n" );
    sky_shader.MoveOnGPU();
    sky_shader.FindUniform( "cu" );
    sky_shader.FindUniform( "cam_pos" );
    sky_shader.FindUniform( "proj_mat" );

    sky_shader.FindAttrib( "coord" );
    sky_shader.SetAttribLocation( "coord", 0 );


    if( shadow_shader.Load( NULL, "shaders/shadow_vert.glsl", NULL ) )
        printf( "error, shadow shader not found\n" );

    shadow_shader.MoveOnGPU();
    shadow_shader.FindUniform( "proj_mat" );

    shadow_shader.FindAttrib( "coord" );
    shadow_shader.SetAttribLocation( "coord", 0 );



    /*if( shadowmap_transform_shader.Load( "shaders/shadowmap_transform_frag.glsl",
                                         "shaders/shadowmap_transform_vert.glsl", NULL ) )
        printf( "shadowmap transform shader not found\n" );

    shadowmap_transform_shader.MoveOnGPU();
    shadowmap_transform_shader.FindUniform( "shadowmap" );
    shadowmap_transform_shader.FindAttrib( "coord" );
    shadowmap_transform_shader.SetAttribLocation( "coord", 0 );*/


    printf( "shaders compiled\n" );

    //raw_sun_shadow_map.Create( 16, 0, NULL, NULL, 1024, 1024 );
    //raw_sun_shadow_map.SetDepthTextureFiltration( GL_NEAREST, GL_NEAREST );
   // raw_sun_shadow_map.DesableDepthTextureCompareMode();
   // raw_sun_shadow_map.BindNull();

    sun_shadow_map.Create( 16, 0, NULL, NULL, 1024, 1024 );
    sun_shadow_map.SetDepthTextureFiltration( GL_NEAREST, GL_LINEAR );
    sun_shadow_map.BindNull();

    // texture_atlas.Load( "textures/textures.bmp" );
    LoadTextures();


    sky_cubemap.Load( "textures/sky/0.bmp" );
    sky_cubemap.SetFiltration( GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR );

    r_InitTextureTable();
    printf( "textures ready\n" );

    vertex_buffer_size= 0;
    vertex_buffer= NULL;

    visibly_world_size[0]= world->NumShreds();//R_SHRED_NUM;
    visibly_world_size[1]= world->NumShreds();//sR_SHRED_NUM;
    shreds= new r_ShredInfo[ visibly_world_size[0] * visibly_world_size[1] ];
    draw_shreds=  new r_ShredInfo[ visibly_world_size[0] * visibly_world_size[1] ];
    /*setup of vertex buffer parametrs*/

    visibly_information= new unsigned char[ visibly_world_size[0] * visibly_world_size[1] * 16 * 16 * 128 ];
    shreds_to_draw_list= new r_ShredInfo*[ visibly_world_size[0] * visibly_world_size[1] ];
    shreds_to_draw_indeces= new quint32[ visibly_world_size[0] * visibly_world_size[1] ];
    shreds_to_draw_quad_count= new quint32[ visibly_world_size[0] * visibly_world_size[1] ];

    printf( "vertex attribs initialized\n" );


    StartUpText();
    printf( "text subsystem initialized" );

    glClearColor( 0.8, 0.8, 1.0, 1.0 );
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glClearDepth( 1.0 );

    glEnable( GL_CULL_FACE );
    glCullFace( GL_FRONT );

    GetExtensionsStrings();
    printf( "\n\n#video information:\n" );
    printf( "vendor:            %s\n", glGetString( GL_VENDOR ) );
    printf( "gl version:        %s\n", glGetString( GL_VERSION ) );

    int i_tmp;
    glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &i_tmp ) ;
    printf( "texture units:     %d\n", i_tmp );
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &i_tmp );
    printf( "max texture size: %d\n", i_tmp );
    glGetIntegerv( GL_MAX_ARRAY_TEXTURE_LAYERS, &i_tmp );
    printf( "max texture layers in array: %d\n", i_tmp );


    //for( int i=0; i< number_of_extension_string; i++ )
    //   printf( "%s\n", extension_strings[i] );
    update_thread.start();
    gpu_data_mutex.unlock();
    need_update_vertex_buffer= false;

    AddNotifyString( "game started" );

}
r_Renderer::r_Renderer( World* w,Player* p, int width, int height ):
    world(w),
    player(p),
    frame_count(0),
    update_count(0),
    fps(0), last_fps_time(0), last_frame_number(0), last_frame_time(0),
    max_fps(0), min_fps(0), max_fps_to_draw(0), min_fps_to_draw(0),
    shreds_update_per_second(0), shreds_update_per_second_to_draw(0),
    startup_time( 0, 0 ),
    gpu_data_mutex(QMutex::NonRecursive),
    host_data_mutex( QMutex::NonRecursive ),
    text_data_mutex( QMutex::NonRecursive ),
    update_thread( SUpdateTick, NULL, true ),
    cam_angle( 0.0, 0.0, 0.0 ),
    fov( 70.0 * m_Math::FM_TORAD ), z_near( 0.5 ), z_far( 512.0 ),
    viewport_x(width), viewport_y(height),
    renderer_initialized( false ), full_update(false), need_full_update_shred_list(false)
{
    world->WriteLock();
    r_Config.double_update_interval_radius= 2;
    r_Config.quad_update_intrval_radius= 3;
    r_Config.octal_update_interval_radius = 4;
    r_Config.update_interval= 50;
    //z_far= float( world->NumShreds() ) * 0.5f * 16.0f;

	notify_log_first_line= R_NUMBER_OF_NOTIFY_LINES+ 1;
    for( int i=0; i< R_NUMBER_OF_NOTIFY_LINES; i++ )
		notify_log[i][0]= 0;
}

bool r_Renderer::GLExtensionSupported( char* ext_name )
{
    unsigned int i;
    unsigned char hash= 0;
    while( ext_name[i] != 0 )
    {
        hash^= ext_name[i];
        i++;
    }
    for( i=0; i< number_of_extension_string; i++ )
    {
        if( hash == extension_strings_hash[i] )
        {
            if( !strcmp( ext_name, extension_strings[i] ) )
                return true;
        }
    }
    return false;
}

char* r_Renderer::extension_strings[ MAX_GL_EXTENSIONS ];
unsigned char r_Renderer::extension_strings_hash[ MAX_GL_EXTENSIONS ];
unsigned int r_Renderer::number_of_extension_string;

void r_Renderer::GetExtensionsStrings()
{
    unsigned int i= 0, j;
    char* str= NULL;
    do
    {
        str= (char*) glGetStringi( GL_EXTENSIONS, i );
        if( str != NULL )
        {
            extension_strings[i]= str;
            extension_strings_hash[i]= 0;
            j= 0;
            while( str[j] != 0 )
            {
                extension_strings_hash[i]^=str[j];
                j++;
            }
        }
        i++;
    }
    while ( str!= NULL && i < MAX_GL_EXTENSIONS );
    number_of_extension_string= i-1;
}

void r_Renderer::BuildShredList()
{
    float fov_x= 2.0 * atan( tan( fov * 0.5 ) * viewport_y / viewport_x );
    unsigned int x, y;
    float a, b;
    a= cam_angle.z + m_Math::FM_PI2;
    b= -cam_angle.x;
    m_Vec3 cam_vec( -cos(a) * cos(b),
                    -sin(a) * cos(b),
                    -sin(b) );

    // AddText( -0.95, -0.9, &m_Vec3( 1.0, 1.0, 1.0 ), 1, "cam vector: %1.2f %1.2f %1.2f", cam_vec.x, cam_vec.y, cam_vec.z );
    a= cam_angle.z + m_Math::FM_PI2;
    b= cam_angle.x + fov * 0.5;
    m_Vec3 top_plane_vec( -cos(a) * sin(b),
                          -sin(a) * sin(b),
                          -cos(b) );

    // AddText( -0.95, -0.85, &m_Vec3( 1.0, 1.0, 1.0 ), 2, "top vector: %1.2f %1.2f %1.2f", top_plane_vec.x, top_plane_vec.y, top_plane_vec.z );

    top_plane_vec.Normalize();
    cam_vec.Normalize();
    float tmp= cam_vec * top_plane_vec * 2.0;
    m_Vec3 bottom_plane_vec= cam_vec * tmp;
    bottom_plane_vec-= top_plane_vec;
    // AddText( -0.95, -0.80, &m_Vec3( 1.0, 1.0, 1.0 ), 2, "bottom vector: %1.2f %1.2f %1.2f", bottom_plane_vec.x, bottom_plane_vec.y, bottom_plane_vec.z );

    m_Vec3 left_plane_vec;
    left_plane_vec.x= cos( cam_angle.z + m_Math::FM_PI - fov_x );
    left_plane_vec.y= -sin( cam_angle.z + m_Math::FM_PI - fov_x );
    m_Vec3 right_plane_vec;
    // m_Vec3 vec_to_shred;
    r_ShredInfo* shred;
    float dot;
    shreds_to_draw_count= 0;
    for( x=0 ; x< visibly_world_size[0]; x++ )
    {
        for( y= 0; y< visibly_world_size[1]; y ++ )
        {
            shred= &draw_shreds[ x + y * visibly_world_size[0] ];
            if( shred->IsOnOtherSideOfPlane( cam_position, cam_vec ) )
            {
                if( shred->IsOnOtherSideOfPlane( cam_position, top_plane_vec ) &&
                        shred->IsOnOtherSideOfPlane( cam_position, bottom_plane_vec ) )
                {
                    //if( shred->IsOnOtherSideOfPlane( cam_position, left_plane_vec ) &&
                    //        shred->IsOnOtherSideOfPlane( cam_position, right_plane_vec ) )
                    //{
                    shreds_to_draw_list[ shreds_to_draw_count ] = shred;
                    shreds_to_draw_indeces[ shreds_to_draw_count ] =shred->index_buffer_offset * 6 * sizeof(quint32);
                    shreds_to_draw_quad_count[ shreds_to_draw_count ]= shred->quad_count * 6;
                    shreds_to_draw_count++;
                    //}
                }
            }
        }
    }
}


void r_Renderer::MoveMap( int dir )
{
    host_data_mutex.lock();

    short x, y, dx, dy, x0, y0, x1, y1, x_shift, y_shift;

    switch (dir)
    {
    case EAST://+x
        x0= 0;
        x1= visibly_world_size[0] - 1;
        y0= 0;
        y1= visibly_world_size[1];
        dx= 1;
        dy= 1;
        x_shift= 1;
        y_shift= 0;
        break;

    case WEST://-x
        x0= visibly_world_size[0] - 1;
        x1= 0;
        y0= 0;
        y1= visibly_world_size[1];
        dx= -1;
        dy= 1;
        x_shift= -1;
        y_shift= 0;
        break;

    case NORTH://+y
        x0= 0;
        x1= visibly_world_size[0];
        y0= visibly_world_size[1] - 1;
        y1= 0;
        dx= 1;
        dy= -1;
        x_shift= 0;
        y_shift= -1;
        break;

    case SOUTH://-y
        x0= 0;
        x1= visibly_world_size[0];
        y0= 0;
        y1= visibly_world_size[1] - 1;
        dx= 1;
        dy= 1;
        x_shift= 0;
        y_shift= 1;
        break;

    default:
        printf( "error. Unknown direction.\n" );
    };

    r_ShredInfo *shred, *shred1;

    r_ShredInfo new_shreds[ 40];
    if( visibly_world_size[0] > 40 || visibly_world_size[1] > 40 )
        printf( "error, shred temp buffer is small." );

    //заполнение данных о новых лоскутах
    switch (dir)
    {
    case WEST:
    case EAST:
        for( y=0; y< visibly_world_size[1]; y++ )
        {
            shred= &new_shreds[y];//&shreds[ x1 + y * visibly_world_size[0] ];
            shred->shred= world->GetShred( x1 * 16, y * 16 );
            shred->longitude= shred->shred->Longitude();
            shred->latitude= shred->shred->Latitude();

            shred->updated= true;
            shred->immediately_update= true;
            shred1= &shreds[ x0 + y * visibly_world_size[0] ];
            shred->quad_buffer_count=   shred1->quad_buffer_count;
            shred->visibly_information= shred1->visibly_information;
            shred->vertex_buffer=       shred1->vertex_buffer;
            shred->index_buffer_offset= shred1->index_buffer_offset;
            shred->GetVisiblyInformation();
            //закольцовывание - буфферы выделенные под старыые лоскуты привязываются к новым
        }
        break;

    case SOUTH:
    case NORTH:
        //printf( "y0=%d, y1=%d\n", y0, y1 );
        for( x=0; x< visibly_world_size[0]; x++ )
        {
            shred= &new_shreds[x];//&shreds[ x + y1 * visibly_world_size[0] ];
            shred->shred= world->GetShred( x * 16, y1 * 16 );
            shred->longitude= shred->shred->Longitude();
            shred->latitude= shred->shred->Latitude();

            shred->updated= true;
            shred->immediately_update= true;
            shred1= &shreds[ x + y0 * visibly_world_size[0] ];
            shred->quad_buffer_count=   shred1->quad_buffer_count;
            shred->visibly_information= shred1->visibly_information;
            shred->vertex_buffer=       shred1->vertex_buffer;
            shred->index_buffer_offset= shred1->index_buffer_offset;
            shred->GetVisiblyInformation();
        }
        break;

    };

    //перемещение лоскутов, уже существующих
    for( x= x0; x != x1; x+= dx )
    {
        for( y= y0; y!= y1; y+=dy )
        {
            shred = &shreds[ x + y * visibly_world_size[0] ];
            shred1= &shreds[ x + x_shift + visibly_world_size[0] * ( y + y_shift ) ];
            memcpy( shred, shred1, sizeof( r_ShredInfo ) );
        }
    }

    //заполнение данных о новых лоскутах
    switch (dir)
    {
    case WEST:
    case EAST:
        for( y=0; y< visibly_world_size[1]; y++ )
            shreds[ x1 + y * visibly_world_size[0] ]= new_shreds[y];
        break;

    case SOUTH:
    case NORTH:
        for( x=0; x< visibly_world_size[0]; x++ )
            shreds[ x + y1 * visibly_world_size[0] ]= new_shreds[x];
        break;

    };
    //выставление указатедей на соседние лоскуты
    for( x=0; x< visibly_world_size[0]; x++ )
    {
        for( y= 0; y< visibly_world_size[1]; y++ )
        {

            shred= &shreds[ x + y * visibly_world_size[0] ];
            //printf( "shred(%d,%d): %d %d\n", x, y, shred->longitude, shred->latitude );
            if( x != 0 )
                shred->west_shred= &shreds[ -1 + x + y * visibly_world_size[0] ];
            else
                shred->west_shred= NULL;

            if( y != 0 )
                shred->north_shred= &shreds[ x + ( y - 1 ) * visibly_world_size[0] ];
            else
                shred->north_shred= NULL;

            if( x != visibly_world_size[0] - 1 )
                shred->east_shred= &shreds[ 1 + x + y * visibly_world_size[0] ];
            else
                shred->east_shred= NULL;

            if( y != visibly_world_size[1] - 1 )
                shred->south_shred= &shreds[ x + ( y + 1 ) * visibly_world_size[0] ];
            else
                shred->south_shred= NULL;

        }
    }

    //обновление соседних с новыми лоскутов, в случае движения на юг или на восток
    if( dir == SOUTH )
    {
        for( x=0; x< visibly_world_size[0]; x++ )
            shreds[ x + ( visibly_world_size[1] - 2 ) * visibly_world_size[0] ].updated= true;
    }
    else if ( dir == EAST )
    {
        for( y=0; y< visibly_world_size[1]; y++ )
            shreds[ visibly_world_size[0] - 2 + y * visibly_world_size[0] ].updated= true;
    }

    Shred* world_shred= world->GetShred( 16 * visibly_world_size[0]/2, 16 * visibly_world_size[1]/2 );
    center_shred_latitude= world_shred->Latitude();
    center_shred_longitude= world_shred->Longitude();



    host_data_mutex.unlock();
}


void r_Renderer::AddNotifyString( const char* str )
{
	memcpy( notify_log[ notify_log_first_line % R_NUMBER_OF_NOTIFY_LINES ],
			 str, min( R_NOTIFY_LINE_LENGTH, strlen( str ) + 1 ) );

	notify_log[ notify_log_first_line % R_NUMBER_OF_NOTIFY_LINES ][ R_NOTIFY_LINE_LENGTH - 1 ]= 0;

	notify_log_first_line++;

}
void r_Renderer::ShutDown()
{
    update_thread.Stop();
}
#endif//RENDERER_CPP
