#include "player.h"
#include "gamedata.h"
#include "smartSprite.h"
#include "viewport.h"
#include "light.h"
#include "levelManager.h"
#include "wall.h"

Player::Player( const std::string& name) :
  player(name),
  initialVelocity(player.getVelocity()),
  observers(std::list<SmartSprite*>()),
  lights(std::vector<Light*>()),
  collisions(),
  worldWidth(Gamedata::getInstance().getXmlInt("world/width")),
  worldHeight(Gamedata::getInstance().getXmlInt("world/height")),
  updateLighting(true){
    addLight(new Light(getPosition()));
}

Player::~Player(){
  for(Light* l : lights) delete l;
}

bool Player::checkWallCollision(Wall* w){
  float right2, left2, top2, bottom2;
  SDL_Rect r;
  r = w->getRect();
  left2 = r.x; right2 = r.x + r.w;
  top2 = r.y; bottom2 = r.y + r.h;
  float left1 = player.getX();

  float right1 = left1+player.getScaledWidth();
  if ( right1 < left2 ) return false;
  if ( left1 > right2 ) return false;
  float top1 = player.getY();

  float bottom1 = top1+player.getScaledHeight();
  if ( bottom1 < top2 ) return false;
  if ( bottom2 < top1 ) return false;
  return true;
}

bool Player::checkForCollisions(){
  // std::vector<Wall*> colls = std::vector<Wall*>(4);
  collisions.clear();
  bool c = false;
  for(auto w : LevelManager::getInstance().getWalls()){
    if(checkWallCollision(w.second)){
      std::cout << "collision: " << w.second->getId() << std::endl;
      collisions.push_back(w.second);
      c = true;
    }
  }
  return c;
}


void Player::stop() {
  player.setVelocity( Vector2f(0, 0) );
  updateLighting = false;
}

void Player::right() {
  if(checkForCollisions()){
    SDL_Rect r;
    for(Wall* w : collisions){
      r = w->getRect();
      // if wall is to the left of player
      std::cout << "wall: " << r.x << ", player: " << player.getX() << std::endl;
      if((r.x < player.getX() + player.getScaledWidth()) &&
      (r.x > player.getX() + player.getScaledWidth()/2)) return;
    }
  }
  if ( player.getX() < worldWidth-getScaledWidth()) {
    player.setVelocityX(initialVelocity[0]);
  }
  updateLighting = true;
}

void Player::left()  {
  if(checkForCollisions()){
    SDL_Rect r;
    for(Wall* w : collisions){
      r = w->getRect();
      // if wall is to the right of player
      std::cout << "Left, wall: " << r.x << ", player: " << player.getX() << std::endl;
      if((r.x > player.getX()) && (r.x < player.getX() + player.getScaledWidth()/2)) return;
    }
  }
  if ( player.getX() > 0) {
    player.setVelocityX(-initialVelocity[0]);
  }
  updateLighting = true;
}

void Player::up()    {
  // std::vector<Wall*> colls = checkForCollisions();
  if(checkForCollisions()){
    SDL_Rect r;
    for(Wall* w : collisions){
      r = w->getRect();
      // if wall is between middle of player and top of player
      if(r.y > player.getY() && r.y < player.getY() + player.getScaledHeight()/2){
        std::cout << "Up, wall: " << r.y << ", player: " << player.getY() << std::endl;
        return;
      }
    }
  }
  if ( player.getY() > 0) {
    player.setVelocityY( -initialVelocity[1] );
  }
  updateLighting = true;
}

void Player::down()  {
  if(checkForCollisions()){
    SDL_Rect r;
    for(Wall* w : collisions){
      r = w->getRect();
      // if wall is between player bottom and player middle
      if(((r.y + r.h) < (player.getY() + player.getScaledHeight())) &&
        ((r.y + r.h) > (player.getY() + player.getScaledHeight()/2))){
        std::cout << "Down, wall: " << (r.y + r.h) << ", player: " << (player.getY() + player.getScaledHeight()) << std::endl;
        return;
      }
    }
  }
  if ( player.getY() < worldHeight-getScaledHeight()) {
    player.setVelocityY( initialVelocity[1] );
  }
  updateLighting = true;
}

void Player::update(Uint32 ticks) {
  player.update(ticks);
  std::list<SmartSprite*>::iterator ptr = observers.begin();
  while ( ptr != observers.end() ) {
    (*ptr)->setPlayerPos( getPosition() );
    ++ptr;
  }
  if(updateLighting){
    for(Light* l: lights){
      l->setPosition(Vector2f(
        getPosition()[0] + getScaledWidth()/2,
        getPosition()[1] + getScaledHeight()/2
      ));
      l->update();
    }


    updateLighting = false;
  }
  stop();
}

void Player::draw() const {
  for(Light* l: lights){
    l->draw();
  }
  player.draw();
}

void Player::detach( SmartSprite *o){
  std::list<SmartSprite*>::iterator ptr = observers.begin();
  while ( ptr != observers.end() ) {
    if ( *ptr == o ) {
      ptr = observers.erase(ptr);
      return;
    }
    ++ptr;
  }
}
