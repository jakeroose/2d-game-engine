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
Move strategy to LevelManager so it can be used by other classes w/out having
to reinstantiate
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
  safeDistance(Gamedata::getInstance().getXmlFloat(name+"/safeDistance")),
  minx(-1),
  maxx(-1),
  alive(true)
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
  safeDistance(Gamedata::getInstance().getXmlFloat(name+"/safeDistance")),
  minx(-1),
  maxx(-1),
  alive(true)
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
  safeDistance(s.safeDistance),
  minx(-1),
  maxx(-1),
  alive(true)
{}

SmartSprite::SmartSprite(const std::string& name) :
  TwoWayMultiSprite(name, 0, 0),
  strategy(new RectangularCollisionStrategy()),
  floor(NULL),
  playerPos(),
  playerWidth(0),
  playerHeight(0),
  currentMode(NORMAL),
  direction(RIGHT),
  safeDistance(Gamedata::getInstance().getXmlFloat(name+"/safeDistance")),
  minx(-1),
  maxx(-1),
  alive(true)
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
      minx = w.second->getMinx();
    }
  }
  if(minx != -1 && getX() <= minx){ isAble = false; }
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
      maxx = getX();
    }
  }
  if(maxx != -1 && getX() >= maxx){ isAble = false; }
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

void SmartSprite::checkForFloor(){
  for(auto w : LevelManager::getInstance().getWalls()){
    if(strategy->collisionBottom(this, w.second) &&
       w.second->getType() == WallType::floor){
      floor = w.second;
      // set min & max x that the enemy should walk between
      maxx = w.second->getMaxx() - 5 - getScaledWidth();
      minx = w.second->getMinx() + 5;
      // reset y so that enemy is level with floor if they fell through a bit
      setY(w.second->getMaxy() - getScaledHeight());
      break;
    }
  }
}

void SmartSprite::update(Uint32 ticks) {
  TwoWayMultiSprite::update(ticks);
  setVelocity(Vector2f(0,0));

  /* if enemy has not made contact with a floor then it should fall until it
     finds one */
  if(floor == NULL){
    goDown();
    checkForFloor();
  } else {
    if(direction == RIGHT){
      goRight();
    } else {
      goLeft();
    }
  }
}

void SmartSprite::kill(){
  alive = false;
  explode();
}

void SmartSprite::reset(){
  alive = true;
  floor = NULL;
  currentMode = NORMAL;
}

/* return distance between player and sprite --- mainly debugging use */
float SmartSprite::getDistance() {
  float x= getX()+getImage()->getWidth()/2;
  float y= getY()+getImage()->getHeight()/2;
  float ex= playerPos[0]+playerWidth/2;
  float ey= playerPos[1]+playerHeight/2;

  return ::distance( x, y, ex, ey );
}
