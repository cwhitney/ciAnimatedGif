#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "ciAnimatedGif.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class BasicSampleApp : public App {
  public:
	void setup() override;
    void keyDown( KeyEvent event) override;
	void mouseDown( MouseEvent event ) override;
    void fileDrop( FileDropEvent event) override;
	void update() override;
	void draw() override;
    
    ciAnimatedGifRef    mGif;
    bool                bDrawPalette = false;
};

void BasicSampleApp::setup()
{
    // from https://en.wikipedia.org/wiki/File:Newtons_cradle_animation_book_2.gif
    mGif = ciAnimatedGif::create( loadAsset("Newtons_cradle_animation_book_2.gif") );
}

void BasicSampleApp::keyDown(cinder::app::KeyEvent event)
{
    if( event.getChar() == 'p'){
        bDrawPalette = !bDrawPalette;
    }
}

void BasicSampleApp::mouseDown( MouseEvent event )
{
    if(mGif){
        mGif->seek(0.0);
    }
}

void BasicSampleApp::fileDrop(cinder::app::FileDropEvent event)
{
    mGif = ciAnimatedGif::create( loadFile(event.getFile(0)) );
}

void BasicSampleApp::update()
{
}

void BasicSampleApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    mGif->draw();
    
    if( bDrawPalette ){
        auto palette = mGif->getPalette();
        for( int i=0; i<palette.size(); i++){
            gl::ScopedColor scCol( palette[i] );
            float s = 10;
            vec2 p((i % 16), floor((float)i / 16.0f));
            gl::drawSolidRect( Rectf( p.x * s, p.y * s, p.x * s + s, p.y * s + s) );
        }
    }

}

CINDER_APP( BasicSampleApp, RendererGl )
