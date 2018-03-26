#include <cmath>
#include <random>
#include <functional>
#include <iostream>
#include "smartSprite.h"
#include "gamedata.h"
#include "renderContext.h"
#include "vector2f.h"

float distance(float x1, float y1, float x2, float y2) {
  float x = x1-x2;
  float y = y1-y2;
  return hypot(x, y);
}

void SmartSprite::goLeft()  { setVelocityX( -abs(getVelocityX()) );  }
void SmartSprite::goRight() { setVelocityX( fabs(getVelocityX()) );  }
void SmartSprite::goUp()    { setVelocityY( -fabs(getVelocityY()) ); }
void SmartSprite::goDown()  { setVelocityY( fabs(getVelocityY()) );  }

/* return distance between player and sprite --- mainly debugging use */
float SmartSprite::getDistance() {
  float x= getX()+getImage()->getWidth()/2;
  float y= getY()+getImage()->getHeight()/2;
  float ex= playerPos[0]+playerWidth/2;
  float ey= playerPos[1]+playerHeight/2;

  return ::distance( x, y, ex, ey );
}

SmartSprite::SmartSprite(const std::string& name, const Vector2f& pos,
  int w, int h) :
  TwoWayMultiSprite(name),
  playerPos(pos),
  playerWidth(w),
  playerHeight(h),
  currentMode(NORMAL),
  safeDistance(Gamedata::getInstance().getXmlFloat(name+"/safeDistance"))
{}

SmartSprite::SmartSprite(const std::string& name, const Vector2f& pos,
  int w, int h, int x, int y) :
  TwoWayMultiSprite(name, x, y),
  playerPos(pos),
  playerWidth(w),
  playerHeight(h),
  currentMode(NORMAL),
  safeDistance(Gamedata::getInstance().getXmlFloat(name+"/safeDistance"))
{}

SmartSprite::SmartSprite(const SmartSprite& s) :
  TwoWayMultiSprite(s),
  playerPos(s.playerPos),
  playerWidth(s.playerWidth),
  playerHeight(s.playerHeight),
  currentMode(s.currentMode),
  safeDistance(s.safeDistance)
{}

void SmartSprite::update(Uint32 ticks) {
  TwoWayMultiSprite::update(ticks);
  float x= getX()+getImage()->getWidth()/2;
  float y= getY()+getImage()->getHeight()/2;
  float ex= playerPos[0]+playerWidth/2;
  float ey= playerPos[1]+playerHeight/2;
  float distanceToEnemy = getDistance();

  if  ( currentMode == NORMAL ) {
    if(distanceToEnemy < safeDistance){
      currentMode = EVADE;
    }
  }
  else if  ( currentMode == EVADE ) {
    if(distanceToEnemy > safeDistance){
      currentMode=NORMAL;
    }
    else {
      if ( x < ex && x > 0) goLeft();
      if ( x > ex && x < worldWidth) goRight();
      if ( y < ey && y > 0) goUp();
      if ( y > ey && y < worldHeight) goDown();
    }
  }
}
