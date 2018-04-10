#include "twowaymultisprite.h"
#include "gamedata.h"
#include "renderContext.h"

TwoWayMultiSprite::TwoWayMultiSprite( const std::string& name) :
  Drawable(name,
           Vector2f((Gamedata::getInstance().getXmlInt(name+"/startLoc/x") + rand() % 30),
                    (Gamedata::getInstance().getXmlInt(name+"/startLoc/y") + rand() % 30)),
           Vector2f((Gamedata::getInstance().getXmlInt(name+"/speedX") + rand() % 100) * (rand() % 2 == 1 ? -1 : 1),
                    (Gamedata::getInstance().getXmlInt(name+"/speedY") + rand() % 100) * (rand() % 2 == 1 ? -1 : 1))
           ),
  images( RenderContext::getInstance()->getImages(name) ),
  currentFrame(0),
  frameOffset(0),
  numberOfFrames( Gamedata::getInstance().getXmlInt(name+"/frames") ),
  frameInterval( Gamedata::getInstance().getXmlInt(name+"/frameInterval")),
  timeSinceLastFrame( 0 ),
  worldWidth(Gamedata::getInstance().getXmlInt("world/width")),
  worldHeight(Gamedata::getInstance().getXmlInt("world/height"))
{ }

TwoWayMultiSprite::TwoWayMultiSprite( const std::string& name, int x, int y) :
  Drawable(name,
           Vector2f((x + rand() % 30),
                    (y + rand() % 30)),
           Vector2f((Gamedata::getInstance().getXmlInt(name+"/speedX") + rand() % 100) * (rand() % 2 == 1 ? -1 : 1),
                    (Gamedata::getInstance().getXmlInt(name+"/speedY") + rand() % 100) * (rand() % 2 == 1 ? -1 : 1))
           ),
  images( RenderContext::getInstance()->getImages(name) ),
  currentFrame(0),
  frameOffset(0),
  numberOfFrames( Gamedata::getInstance().getXmlInt(name+"/frames") ),
  frameInterval( Gamedata::getInstance().getXmlInt(name+"/frameInterval")),
  timeSinceLastFrame( 0 ),
  worldWidth(Gamedata::getInstance().getXmlInt("world/width")),
  worldHeight(Gamedata::getInstance().getXmlInt("world/height"))
{ }

TwoWayMultiSprite::TwoWayMultiSprite(const TwoWayMultiSprite& s) :
  Drawable(s),
  images(s.images),
  currentFrame(s.currentFrame),
  frameOffset(s.frameOffset),
  numberOfFrames( s.numberOfFrames ),
  frameInterval( s.frameInterval ),
  timeSinceLastFrame( s.timeSinceLastFrame ),
  worldWidth( s.worldWidth ),
  worldHeight( s.worldHeight )
{ }

void TwoWayMultiSprite::advanceFrame(Uint32 ticks) {
	timeSinceLastFrame += ticks;
	if (timeSinceLastFrame > frameInterval) {
    currentFrame = (((currentFrame+1) % (numberOfFrames / 2)) + frameOffset) % numberOfFrames;
		timeSinceLastFrame = 0;
	}
}

TwoWayMultiSprite& TwoWayMultiSprite::operator=(const TwoWayMultiSprite& s) {
  Drawable::operator=(s);
  images = (s.images);
  currentFrame = (s.currentFrame);
  frameOffset = (s.frameOffset);
  numberOfFrames = ( s.numberOfFrames );
  frameInterval = ( s.frameInterval );
  timeSinceLastFrame = ( s.timeSinceLastFrame );
  worldWidth = ( s.worldWidth );
  worldHeight = ( s.worldHeight );
  return *this;
}

void TwoWayMultiSprite::draw() const {
  images[currentFrame]->draw(getX(), getY(), getScale());
}

void TwoWayMultiSprite::update(Uint32 ticks) {
  advanceFrame(ticks);

  Vector2f incr = getVelocity() * static_cast<float>(ticks) * 0.001;
  setPosition(getPosition() + incr);

  // if ( getY() < 0) {
  //   setVelocityY( fabs( getVelocityY() ) );
  // }
  // if ( getY() > worldHeight-getScaledHeight()) {
  //   setVelocityY( -fabs( getVelocityY() ) );
  // }
  //
  // if ( getX() < 0) {
  //   setVelocityX( fabs( getVelocityX() ) );
  // }
  // if ( getX() > worldWidth-getScaledWidth()) {
  //   setVelocityX( -fabs( getVelocityX() ) );
  // }

  if(getVelocityX() < 0){
    frameOffset = numberOfFrames / 2;
  } else {
    frameOffset = 0;
  }

}
