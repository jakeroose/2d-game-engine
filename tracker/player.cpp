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
  state(PlayerState::falling),
  worldWidth(Gamedata::getInstance().getXmlInt("world/width")),
  worldHeight(Gamedata::getInstance().getXmlInt("world/height")),
  updateLighting(true){
    addLight(new Light(getPosition()));
    checkForCollisions();
}

Player::~Player(){
  for(Light* l : lights) delete l;
}

bool Player::checkWallCollision(Wall* w){
  float right2, left2, top2, bottom2;
  int tolerance = 10;
  bool wall = (w->getType() == WallType::wall);
  SDL_Rect r;
  r = w->getRect();
  left2 = r.x +         (wall ? 0 : tolerance);
  right2 = r.x + r.w +  (wall ? 0 : -tolerance);
  top2 = r.y +          (wall ? tolerance : 0);
  bottom2 = r.y + r.h + (wall ? -tolerance : 0);
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

bool Player::collisionRight(Wall* w){
  SDL_Rect r = w->getRect();
    // if wall is between player middle and player right
  std::cout << "wall: " << r.x << ", player: " << player.getX() << std::endl;
  if(w->getType() == WallType::wall &&
    (r.x < player.getX() + player.getScaledWidth()) &&
    (r.x > player.getX() + player.getScaledWidth()/2)) return true;
  return false;
}

bool Player::collisionLeft(Wall* w){
  SDL_Rect r = w->getRect();
  // if wall is between player middle and player right
  std::cout << "Left, wall: " << r.x << ", player: " << player.getX() << std::endl;
  if(w->getType() == WallType::wall &&
    (r.x > player.getX()) &&
    (r.x < player.getX() + player.getScaledWidth()/2)) return true;
  return false;
}

bool Player::collisionTop(Wall* w){
  SDL_Rect r = w->getRect();
  // if wall is between middle of player and top of player
  std::cout << "Up, wall: " << r.y << ", player: " << player.getY() << std::endl;
  if(w->getType() == WallType::floor &&
     r.y > player.getY() &&
     r.y < player.getY() + player.getScaledHeight()/2) return true;
  return false;
}

bool Player::collisionBottom(Wall* w){
  SDL_Rect r = w->getRect();
  // if wall is between player bottom and player middle
  std::cout << "Down, wall: " << (r.y + r.h) << ", player: " <<
   (player.getY() + player.getScaledHeight()) << std::endl;
  if(w->getType() == WallType::floor &&
    ((r.y + r.h) < (player.getY() + player.getScaledHeight())) &&
    ((r.y + r.h) > (player.getY() + player.getScaledHeight()/2))) return true;
  return false;
}

void Player::right() {
  if(checkForCollisions()){
    for(Wall* w : collisions){
      if(collisionRight(w)) return;
    }
  }
  if ( player.getX() < worldWidth-getScaledWidth()) {
    player.setVelocityX(initialVelocity[0]);
  }
  updateLighting = true;
}

void Player::left()  {
  if(checkForCollisions()){
    for(Wall* w : collisions){
      if(collisionLeft(w)) return;
    }
  }
  if ( player.getX() > 0) {
    player.setVelocityX(-initialVelocity[0]);
  }
  updateLighting = true;
}

void Player::up()    {
  // state = PlayerState::jumping;
  if(checkForCollisions()){
    for(Wall* w : collisions){
      if(collisionTop(w)) return;
    }
  }
  if ( player.getY() > 0) {
    player.setVelocityY( -initialVelocity[1] );
  }
  updateLighting = true;
}

void Player::down()  {
  if(checkForCollisions()){
    for(Wall* w : collisions){
      if(collisionBottom(w)) return;
    }
  }
  if ( player.getY() < worldHeight-getScaledHeight()) {
    player.setVelocityY( initialVelocity[1] );
  }
  updateLighting = true;
}

void Player::updatePlayerState(){
  updateLighting = true;
  if(player.getVelocityY() > 0 || collisions.size() == 0){
    state = PlayerState::falling;
  } else if(player.getVelocityY() < 0){
    state = PlayerState::jumping;
  } else if(player.getVelocityX() != 0){
    state = PlayerState::walking;
    player.setVelocityX(0);
  } else {
    state = PlayerState::idle;
    updateLighting = false;
    // stop();
  }
}

void Player::update(Uint32 ticks) {
  player.update(ticks);
  std::list<SmartSprite*>::iterator ptr = observers.begin();
  while ( ptr != observers.end() ) {
    (*ptr)->setPlayerPos( getPosition() );
    ++ptr;
  }

  updatePlayerState();

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

  float gravity = 10;

  // TODO: Don't do check if player is on a floor. Currently makes them get
  // stuck on walls though bc of updatePlayerState().
  // Here down should probably all be in update state.
  // if(state != PlayerState::idle){
  checkForCollisions();
  // check if they should be falling
  for(Wall* w : collisions){
    if(collisionBottom(w)){
      state = PlayerState::idle;
      stop();
      break;
    } else {
      state = PlayerState::falling;
      std::cout << "falling" << std::endl;
    }
    if(collisionTop(w)){
      player.setVelocityY(1);
    }
    if(collisionRight(w) || collisionLeft(w)){
      player.setVelocityX(0);
    }
  }
  // }
  if(state != PlayerState::idle){
    player.setVelocityY(player.getVelocityY() + gravity);
  } else {
    stop();
  }

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
