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

#ifndef SCREEN_FUNC_H
#define SCREEN_FUNC_H

#include <QString>
#include <QTimer>

#include "screen.h"
#include "world.h"
#include "blocks.h"
#include "Player.h"

enum Qt::Key;


FREGGLWidget::FREGGLWidget( Screen* s, r_Renderer* screen_renderer, QGLFormat format, QWidget* parent ):
    QGLWidget( format, parent )
{
    renderer=screen_renderer;
    screen= s;
    QGLFormat f= context()->format();

    if( FREG_GL_VERSION_MAJOR > format.majorVersion() )
        printf( "error. OpenGL %d.%d does no supported\n", FREG_GL_VERSION_MAJOR, FREG_GL_VERSION_MINOR );
    else if( FREG_GL_VERSION_MAJOR == format.majorVersion() && FREG_GL_VERSION_MINOR > format.minorVersion() )
        printf( "error. OpenGL %d.%d does no supported\n", FREG_GL_VERSION_MAJOR, FREG_GL_VERSION_MINOR );
}

FREGGLWidget::~FREGGLWidget() {}


QSize FREGGLWidget::minimumSizeHint() const
{
    return QSize(renderer->ViewportWidth(), renderer->ViewportHeight() );
}

QSize FREGGLWidget::sizeHint() const
{
    return QSize(renderer->ViewportWidth(), renderer->ViewportHeight() );
}

void FREGGLWidget::initializeGL()
{
    renderer->Initialize();
}

void FREGGLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height );
}

void FREGGLWidget::paintGL()
{
    renderer->Draw();
    update();
}

void FREGGLWidget::mousePressEvent(QMouseEvent* e)
{
    screen->mousePressEvent(e);
}
void FREGGLWidget::mouseMoveEvent(QMouseEvent* e)
{
    screen->mouseMoveEvent(e);
}
void FREGGLWidget::keyPressEvent(QKeyEvent* e)
{
    screen->keyPressEvent(e);
}
void FREGGLWidget::keyReleaseEvent(QKeyEvent* e)
{
    screen->keyReleaseEvent(e);
}
void FREGGLWidget::closeEvent(QCloseEvent* e)
{
    screen->closeEvent(e);
}



void Screen::PassString(QString & str) const
{
}

void Screen::Print()
{
}


void Screen::Notify(const QString& str)
{
    printf( "notify: %s\n", str.toLocal8Bit().constData() );
    renderer->AddNotifyString( str.toLocal8Bit().constData()  );
}

void Screen::UpdateAround(
    const ushort x ,
    const ushort y,
    const ushort z,
    const ushort range)
{

    if( ! renderer->Initialized() )
        return;

    short x0, y0, z0, x1, y1, z1;
    x0= max( 0, x - range );
    x1= min( x + range, w->NumShreds() * R_SHRED_WIDTH - 1 );

    y0= max( 0, y - range );
    y1= min( y + range, w->NumShreds() * R_SHRED_WIDTH - 1 );

    z0= max( 0, z - range );
    z1= min( z + range, R_SHRED_HEIGHT - 2 );

    renderer->UpdateCube( x0, y0, z0, x1, y1, z1 );
    //printf( "update cube: %d %d %d\n", x, y, z );

}
void Screen::Update(
    const unsigned short x,
    const unsigned short y,
    const unsigned short z)
{
   if( renderer->Initialized() )
    {
        renderer->UpdateBlock( x, y, z );
        //printf( "update\n" );
    }
}
void Screen::UpdateAll()
{
   if( renderer->Initialized() )
    {
        renderer->UpdateAll();
        printf( "update all\n" );
    }
}

Screen::Screen(
    World * const wor,
    Player * const pl):
    updated(true),
    cleaned(false),
    //notifyLines(0),
    cam_ang( 0.0, 0.0, 0 ),
    cam_pos( 0.0, 0.0, 55.0 ),
    renderer( NULL ),
    input_thread( sInputTick, NULL, true ),
    VirtScreen( wor, pl ),
    screen_width(1024), screen_height(768),
    startup_time( 0, 0 )
{
    renderer= new r_Renderer( wor,pl, screen_width, screen_height );
    QGLFormat format;
    format.setVersion( FREG_GL_VERSION_MAJOR, FREG_GL_VERSION_MINOR );//format.setVersion( 3, 2 );
    format.setProfile( QGLFormat::NoProfile );

    format.setSwapInterval(1);
    gl_widget= new FREGGLWidget( this, renderer, format, NULL );
    layout=new QVBoxLayout();
    window= new QWidget( NULL, 0 );

    window->setLayout(layout);
    window->move( 0, 0 );
    layout->addWidget( gl_widget );
    layout->setMargin(0);
    gl_widget->setFixedSize( screen_width, screen_height );

    window->setFocusPolicy( Qt::StrongFocus );
    gl_widget->setFocusPolicy( Qt::StrongFocus );
    gl_widget->setFocus();

    window->show();
    window->setFixedSize( window->size() );

    for( unsigned int i= 0; i< 512; i++ )
        keys[i]= false;
    use_mouse= false;

    current_screen= this;

    connect(this, SIGNAL(InputReceived(int, int)),
            player, SLOT(Act(int, int)),
            Qt::DirectConnection);


    input_thread.start();
}

Screen::~Screen()
{
    //CleanAll();
}

void Screen::CleanAll()
{
}





Screen* Screen::current_screen= NULL;


void Screen::mousePressEvent(QMouseEvent * e)
{
}
void Screen::mouseMoveEvent(QMouseEvent * e)
{
}
void Screen::keyPressEvent( QKeyEvent* e )
{
    int key= e->key();
    key= ( key & 0xff ) | ( key >> 16 );
    if( key < 512 )
        keys[ key ]= true;
    switch(key)
    {


    case Qt::Key_O:
        InputReceived(MOVE, NORTH );
        InputReceived( TURN, NORTH );
        break;

    case Qt::Key_L:
        InputReceived(MOVE, SOUTH );
        InputReceived( TURN, SOUTH );
        break;

    case Qt::Key_K:
        InputReceived(MOVE, WEST );
        InputReceived( TURN, WEST );
        break;

    case Qt::Key_Semicolon:
        InputReceived(MOVE, EAST );
        InputReceived( TURN, EAST);
        break;

    case Qt::Key_X:
        InputReceived( JUMP, HERE );
        break;

	case Qt::Key_Z:
        InputReceived( EXAMINE, HERE );
        break;

    case Qt::Key_Q:
        renderer->ShutDown();
        ExitReceived();
        break;

    default:
        break;
    };
}
void Screen::closeEvent(QCloseEvent *)
{
    renderer->ShutDown();
    ExitReceived();
}
void Screen::keyReleaseEvent( QKeyEvent* e )
{
    int key= e->key();
    key= ( key & 0xff ) | ( key >> 16 );
    if( key < 512 )
        keys[ key ]= false;
}
void Screen::InputTick()
{
    while( renderer == NULL )
        usleep( 1000 );
    while( !renderer->Initialized() )
        usleep( 1000 );

    static int last_time= 0;
    int dt_i= -QTime::currentTime().msecsTo( startup_time ) - last_time;
    float dt= float( dt_i );
    /*if( use_mouse )
    {
        POINT p;
        GetCursorPos( &p );
        int dx, dy;
        dx= p.x - 256;
        dy= p.y - 256;

        cam_ang.z += (float)dx * 0.005;
        cam_ang.x -= (float)dy * 0.005;

        if( cam_ang.x > m_Math::FM_PI2 ) cam_ang.x= m_Math::FM_PI2;
        else if( cam_ang.x < -m_Math::FM_PI2 ) cam_ang.x= -m_Math::FM_PI2;

        if( cam_ang.z > m_Math::FM_PI ) cam_ang.z-= m_Math::FM_2PI;
        else if( cam_ang.z < -m_Math::FM_PI ) cam_ang.z+= m_Math::FM_2PI;

        p.x= 256;
        p.y= 256;
        SetCursorPos( p.x, p.y );
    }*/
    if( keys[ FREG_KEY_FORWARD ] )
    {
        cam_pos.y-= dt * 0.025 * m_Math::Cos( cam_ang.z );
        cam_pos.x+= dt * 0.025 * m_Math::Sin( cam_ang.z );
    }
    if( keys[ FREG_KEY_BACKWARD ] )
    {
        cam_pos.y+= dt * 0.025 * m_Math::Cos( cam_ang.z );
        cam_pos.x-= dt * 0.025 * m_Math::Sin( cam_ang.z );
    }

    if( keys[ FREG_KEY_STRAFE_LEFT ] )
    {
        cam_pos.y-= dt * 0.025 * m_Math::Cos( cam_ang.z - m_Math::FM_PI2 );
        cam_pos.x+= dt * 0.025 * m_Math::Sin( cam_ang.z - m_Math::FM_PI2);
    }
    if( keys[ FREG_KEY_STRAFE_RIGHT ] )
    {
        cam_pos.y-= dt * 0.025 * m_Math::Cos( cam_ang.z + m_Math::FM_PI2);
        cam_pos.x+= dt * 0.025 * m_Math::Sin( cam_ang.z + m_Math::FM_PI2);
    }

    if( keys[ FREG_KEY_UP ] )
    {
        cam_ang.x+= dt * 0.18 * m_Math::FM_TORAD;
        if( cam_ang.x > m_Math::FM_PI2 ) cam_ang.x= m_Math::FM_PI2;
    }
    if( keys[ FREG_KEY_DOWN ] )
    {
        cam_ang.x-= dt * 0.18 * m_Math::FM_TORAD;
        if( cam_ang.x < -m_Math::FM_PI2 ) cam_ang.x= -m_Math::FM_PI2;
    }

    if( keys[ FREG_KEY_LEFT ] )
    {
        cam_ang.z-= dt * 0.18 * m_Math::FM_TORAD;
        if( cam_ang.z < -m_Math::FM_PI ) cam_ang.z+= m_Math::FM_2PI;
    }
    if( keys[ FREG_KEY_RIGHT ] )
    {
        cam_ang.z+= dt * 0.18 * m_Math::FM_TORAD;
        if( cam_ang.z > m_Math::FM_PI ) cam_ang.z-= m_Math::FM_2PI;

    }

    if( keys[ FREG_KEY_JUMP ] )
    {
        cam_pos.z+= dt * 0.025;
    }
    if( keys[ FREG_KEY_CROUCH ] )
    {
        cam_pos.z-= dt * 0.025;
    }
    renderer->SetCamAngle( cam_ang );
    renderer->SetCamPosition( cam_pos );
    usleep( 30 * 1000 );
    last_time+= dt_i;
}
#endif
