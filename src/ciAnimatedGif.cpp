#include "ciAnimatedGif.h"

using namespace ci;
using namespace ci::app;
using namespace std;

ciAnimatedGif::ciAnimatedGif(ci::DataSourceRef data)
{
    parseMetadata(data);
    
    ci::ImageSource::Options options;
    options.index( 0 );
    
    ImageSourceRef img = loadImage( data, options );
    int numFrames = img->getCount();
    
    for( int i = 0; i < numFrames; ++i ) {
        SurfaceRef frame = Surface::create( loadImage(data, options) );
        mFrameList.push_back( gl::Texture::create(*frame) );
        options.index( options.getIndex() + 1 );
    }
    
    mCurFrame = 0;
    play();
}

void ciAnimatedGif::play()
{
    mNextFrameTime = getElapsedSeconds() + mFrameTimes[mCurFrame];
}

void ciAnimatedGif::seek(float pct)
{
    mCurFrame = (mFrameList.size() - 1) * glm::clamp(pct, 0.0f, 1.0f);
}

void ciAnimatedGif::parseMetadata(ci::DataSourceRef data)
{
    BufferRef buffer = data->getBuffer();
    uint8_t * inPtr = (uint8_t *)buffer->getData();
 
    // HEADER BLOCK (must start with GIF)
    char header[6];
    memcpy(&header, inPtr, 6);
    mHeader = std::string(header);
    if( mHeader.substr(0,3) != "GIF" ){
        throw new Exception("This is not a valid animated gif file.");
    }
    
    // LOGICAL SCREEN DESCRIPTOR
    canvasWidth    = *(inPtr + 6) | *(inPtr + 7) << 8;  // Not guaranteed to be accurate. Often returns 0
    canvasHeight   = *(inPtr + 8) | *(inPtr + 9) << 8;  // Not guaranteed to be accurate. Often returns 0
    
    uint8_t packed = *(inPtr + 10);
    
    bool hasGlobalColorTable = (packed & 0x01);
    if(hasGlobalColorTable){
        int mask = (1 << 6) | (1 << 5) | (1 << 4);
        int n = (mask & packed) >> 4;
        int numColorBytes = 3 * pow(2, n + 1);
        
        uint8_t *colPtr = inPtr + 13;
        
        for( int i=0; i<numColorBytes-1; i++){
            mColorList.push_back( Color::hex( *(colPtr + i) << 16 | *(colPtr + i + 1) << 8 | *(colPtr + i + 2) ) );
            i += 2;
        }
    }
    
    // Find application extension blocks
    for(int i=0; i<buffer->getSize(); i++){
        if( *(inPtr + i) == 0x21 ){                                     // it's an extension
            if( *(inPtr + i + 1) == 0xF9 && *(inPtr + i + 7) == 0x00){  // it's a graphics control label
                uint8_t byte1 = *(inPtr + i + 4);
                uint8_t byte2 = *(inPtr + i + 5);
                
                uint16_t sum = byte1 | (byte2 << 8);    // this represents the delay in hundreths of a second
                if( sum == 0.0 ){
                    sum = 10;      // if no delay specificed, put a default delay of 0.1 secs between frames
                }
                
                mFrameTimes.push_back( (float)sum / 100.0f );
            }
        }
    }
}

void ciAnimatedGif::printBit(uint8_t byte)
{
    cout    << ((byte >> 7) & 0x1)
            << ((byte >> 6) & 0x1)
            << ((byte >> 5) & 0x1)
            << ((byte >> 4) & 0x1)
            << ((byte >> 3) & 0x1)
            << ((byte >> 2) & 0x1)
            << ((byte >> 1) & 0x1)
            << ((byte) & 0x1)
            << endl;
}

void ciAnimatedGif::update()
{
    while( getElapsedSeconds() > mNextFrameTime ){
        ++mCurFrame;
        if( mCurFrame >= mFrameList.size() ){
            mCurFrame = 0;
        }

        mNextFrameTime += mFrameTimes[glm::clamp<int>(mCurFrame, 0, mFrameTimes.size()-1)];
    }
    mCurTex = mFrameList[mCurFrame];
}

void ciAnimatedGif::draw()
{
    update();

    gl::draw(mCurTex);
    
}
