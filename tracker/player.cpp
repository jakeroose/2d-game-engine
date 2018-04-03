#include "player.h"
#include "gamedata.h"
#include "smartSprite.h"
#include "viewport.h"
#include "light.h"
#include "levelManager.h"
#include "wall.h"
#include "collectable.h"
#include "clock.h"
#include "lightRenderer.h"

std::ostream& operator<<(std::ostream& out, PlayerState s){
  std::string state = "idk";
  if(s == PlayerState::idle){
    state = "idle";
  } else if(s == PlayerState::falling){
    state = "falling";
  } else if(s == PlayerState::jumping){
    state = "jumping";
  } else if(s == PlayerState::walking){
    state = "walking";
  }
  return out << state;
}

const std::string Player::getStateStr(){
  std::string s = "idk";
  if(state == PlayerState::idle){
    s = "idle";
  } else if(state == PlayerState::falling){
    s = "falling";
  } else if(state == PlayerState::jumping){
    s = "jumping";
  } else if(state == PlayerState::walking){
    s = "walking";
  }
  return s;
}


Player::Player( const std::string& name) :
  player(name),
  initialVelocity(player.getVelocity()),
  observers(std::list<SmartSprite*>()),
  lights(std::vector<Light*>()),
  collisions(),
  collectables(std::vector<Collectable*>()),
  state(PlayerState::falling),
  worldWidth(Gamedata::getInstance().getXmlInt("world/width")),
  worldHeight(Gamedata::getInstance().getXmlInt("world/height")),
  updateLighting(true),
  hoverHeight(LevelManager::UNIT_SIZE/4),
  energy(1),
  flyPower(LevelManager::UNIT_SIZE),
  totalEnergies(1)
  {
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

  float bottom1 = top1+player.getScaledHeight() + hoverHeight;
  if ( bottom1 < top2 ) return false;
  if ( bottom2 < top1 ) return false;
  return true;
}

bool Player::checkForCollisions(){
  collisions.clear();
  bool c = false;
  for(auto w : LevelManager::getInstance().getWalls()){
    if(checkWallCollision(w.second)){
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
  if(w->getType() == WallType::wall &&
    (r.x < player.getX() + player.getScaledWidth()) &&
    (r.x > player.getX() + player.getScaledWidth()/2)) return true;
  return false;
}

bool Player::collisionLeft(Wall* w){
  SDL_Rect r = w->getRect();
  // if wall is between player middle and player right
  if(w->getType() == WallType::wall &&
    (r.x > player.getX()) &&
    (r.x < player.getX() + player.getScaledWidth()/2)) return true;
  return false;
}

bool Player::collisionTop(Wall* w){
  SDL_Rect r = w->getRect();
  // if wall is between middle of player and top of player
  if(w->getType() == WallType::floor &&
     r.y > player.getY() &&
     r.y < player.getY() + player.getScaledHeight()/2) return true;
  return false;
}

bool Player::collisionBottom(Wall* w){
  SDL_Rect r = w->getRect();
  // if wall is between player bottom and player middle
  if(w->getType() == WallType::floor &&
    ((r.y + r.h) < (player.getY() + player.getScaledHeight() + hoverHeight)) &&
    ((r.y + r.h) > (player.getY() + player.getScaledHeight()/2 + hoverHeight))) return true;
  return false;
}

void Player::right() {
  state = PlayerState::walking;
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
  state = PlayerState::walking;
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
  up(1);
}


// tick based approach fails when frame rates drop :/
void Player::up(Uint32 ticks)    {
  state = PlayerState::jumping;
  if(checkForCollisions()){
    for(Wall* w : collisions){
      if(collisionTop(w)) return;
    }
  }

  // for smooth falling, if energy == 0 then subtract velocity and cap it at
  // flyPower, instead of hard setting it.

  // reduce energy by flyPower and set YVelocity
  if(player.getY() > 0 && useEnergy((int)ticks)){
    player.setVelocityY( -(int)flyPower);
  } else {
    player.setVelocityY(flyPower);
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
  if(player.getVelocityY() > 0){
    state = PlayerState::falling;
  } else if(player.getVelocityY() < 0){
    state = PlayerState::jumping;
  } else if(player.getVelocityX() != 0){
    state = PlayerState::walking;
    player.setVelocityX(0);
  } else {
    state = PlayerState::idle;
    updateLighting = false;
  }
}

void Player::handleGravity(){
  float gravity = 10; // cool constant you have there..

  // Here down should probably all be in update state.
  checkForCollisions();
  // check if they should be falling
  bool falling = true;
  for(Wall* w : collisions){
    if(collisionBottom(w)){
      falling = false;
      refillEnergy();
    }
    if(collisionTop(w)){
      player.setVelocityY(0);
    }
    if(collisionRight(w) || collisionLeft(w)){
      player.setVelocityX(0);
    }
  }

  // "That wasn't flying, that was falling with style."
  // (could be flying or falling)
  if(falling){
    player.setVelocityY(player.getVelocityY() + gravity);
    // may get set to 0, in which case player may not realize they're falling
    if(player.getVelocityY() == 0) player.setVelocityY(1);
    // player shouldn't float from left to right when falling with style
    player.setVelocityX(0);
  } else {
    player.setVelocityY(0);
  }
}

void Player::refillEnergy(){
  energy = maxEnergy();
  for(Light* l: lights){
    l->setIntensity(l->getBaseIntensity());
  }
}

// reduces energy by i, returns energy remaining
int Player::useEnergy(int i) {
  energy -= i;
  if(energy < 0) energy = 0;
  return energy;
}


int Player::maxEnergy(){
  return LevelManager::UNIT_SIZE*8*totalEnergies;
}

void Player::addCollectable(Collectable* c) {
  collectables.push_back(c);
  totalEnergies += 1;
}

void Player::update(Uint32 ticks) {
  player.update(ticks);
  std::list<SmartSprite*>::iterator ptr = observers.begin();
  while ( ptr != observers.end() ) {
    (*ptr)->setPlayerPos( getPosition() );
    ++ptr;
  }

  handleGravity();
  updatePlayerState();

  if(updateLighting){
    for(Light* l: lights){
      l->setPosition(Vector2f(
        getPosition()[0] + getScaledWidth()/2,
        getPosition()[1] + getScaledHeight()/2
      ));
      l->update();
      // scale light based on energy left, keep a minimum of 1/5 intensity
      l->setIntensity(l->getBaseIntensity()*
                      (1+(4.0*energy/maxEnergy()))/5.0);
    }
    updateLighting = false;
  }

  for(int i = 0; i < (int)collectables.size(); i++){
    // double angle = Clock::getInstance().getTicks()*2*3.14*i;
    float angle = Clock::getInstance().getTicks()*0.001 + i*10;
    collectables[i]->setPosition(Vector2f(cos(angle), sin(angle))*25 + getPosition());
  }

}

void Player::draw() const {
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
