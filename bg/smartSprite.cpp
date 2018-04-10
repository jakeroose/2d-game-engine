#include <cmath>
#include <random>
#include <functional>
#include <iostream>
#include "smartSprite.h"
#include "gamedata.h"
#include "renderContext.h"
#include "vector2f.h"
#include "levelManager.h"

/*
TODO:
Optimization can be made to set max wander distance based on floor and then
collision with left and right walls, also to make sure enemies don't fall.

*/

SmartSprite::SmartSprite(const std::string& name, const Vector2f& pos,
  int w, int h) :
  TwoWayMultiSprite(name),
  strategy(new RectangularCollisionStrategy()),
  floor(NULL),
  playerPos(pos),
  playerWidth(w),
  playerHeight(h),
  currentMode(NORMAL),
  direction(RIGHT),
  safeDistance(Gamedata::getInstance().getXmlFloat(name+"/safeDistance"))
{}

SmartSprite::SmartSprite(const std::string& name, const Vector2f& pos,
  int w, int h, int x, int y) :
  TwoWayMultiSprite(name, x, y),
  strategy(new RectangularCollisionStrategy()),
  floor(NULL),
  playerPos(pos),
  playerWidth(w),
  playerHeight(h),
  currentMode(NORMAL),
  direction(RIGHT),
  safeDistance(Gamedata::getInstance().getXmlFloat(name+"/safeDistance"))
{}

SmartSprite::SmartSprite(const SmartSprite& s) :
  TwoWayMultiSprite(s),
  strategy(new RectangularCollisionStrategy()),
  floor(s.floor),
  playerPos(s.playerPos),
  playerWidth(s.playerWidth),
  playerHeight(s.playerHeight),
  currentMode(s.currentMode),
  direction(s.direction),
  safeDistance(s.safeDistance)
{}

SmartSprite::~SmartSprite(){
  delete strategy;
}

int speed = 100;

float distance(float x1, float y1, float x2, float y2) {
  float x = x1-x2;
  float y = y1-y2;
  return hypot(x, y);
}

void SmartSprite::goLeft()  {
  bool isAble = true;
  for(auto w: LevelManager::getInstance().getWalls()){
    if(strategy->collisionLeft(this, w.second)){
      isAble = false;
    }
  }
  if(isAble){
    setVelocityX( -speed );
  } else {
    direction = RIGHT;
  }
}
void SmartSprite::goRight() {
  bool isAble = true;
  for(auto w: LevelManager::getInstance().getWalls()){
    if(strategy->collisionRight(this, w.second)){
      isAble = false;
    }
  }
  if(isAble){
    setVelocityX( speed );
  } else {
    direction = LEFT;
  }
}
void SmartSprite::goUp()    {
  bool isAble = true;
  for(auto w: LevelManager::getInstance().getWalls()){
    if(strategy->collisionTop(this, w.second)){
      isAble = false;
    }
  }
  if(isAble)
  setVelocityY( -speed );
}

void SmartSprite::goDown()  {
  bool isAble = true;
  for(auto w: LevelManager::getInstance().getWalls()){
    if(strategy->collisionBottom(this, w.second)){
      isAble = false;
    }
  }
  if(isAble)
  setVelocityY( speed );
}

/* return distance between player and sprite --- mainly debugging use */
float SmartSprite::getDistance() {
  float x= getX()+getImage()->getWidth()/2;
  float y= getY()+getImage()->getHeight()/2;
  float ex= playerPos[0]+playerWidth/2;
  float ey= playerPos[1]+playerHeight/2;

  return ::distance( x, y, ex, ey );
}



void SmartSprite::update(Uint32 ticks) {
  TwoWayMultiSprite::update(ticks);
  float x= getX()+getImage()->getWidth()/2;
  float y= getY()+getImage()->getHeight()/2;
  float ex= playerPos[0]+playerWidth/2;
  float ey= playerPos[1]+playerHeight/2;
  float distanceToEnemy = getDistance();
  setVelocity(Vector2f(0,0));

  /* if enemy has not made contact with a floor then it should fall until it
     finds one */
  if(floor == NULL){
    goDown();
    for(auto w : LevelManager::getInstance().getWalls()){
      if(strategy->collisionBottom(this, w.second) &&
         w.second->getType() == WallType::floor){
        floor = w.second;
        break;
      }
    }
  } else {
    if(direction == RIGHT){
      goRight();
    } else {
      goLeft();
    }
  }


  // always wandering back and forth for now. different enemies may have diff
  // behaviors.
  if  ( currentMode == NORMAL ) {
    // if(distanceToEnemy < safeDistance){
    //   currentMode = ATTACK;
    // }
  }
  else if  ( currentMode == EVADE ) {
    if(distanceToEnemy > safeDistance){
      currentMode=NORMAL;
    }
    else {
      if ( x < ex && x > 0) goLeft();
      if ( x > ex && x < worldWidth) goRight();
    }
  }
  else if  ( currentMode == ATTACK ) {
    if(distanceToEnemy > safeDistance){
      currentMode=NORMAL;
    }
    else {
      if ( x < ex && x > 0) goRight();
      if ( x > ex && x < worldWidth) goLeft();
    }
  }
}
