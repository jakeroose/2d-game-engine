#include "player.h"
#include "gamedata.h"
#include "smartSprite.h"
#include "viewport.h"
#include "light.h"
#include "levelManager.h"
#include "wall.h"

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
  state(PlayerState::falling),
  worldWidth(Gamedata::getInstance().getXmlInt("world/width")),
  worldHeight(Gamedata::getInstance().getXmlInt("world/height")),
  updateLighting(true),
  hoverHeight(LevelManager::UNIT_SIZE/4),
  energy(LevelManager::UNIT_SIZE*4),
  flyPower(initialVelocity[1]),
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
      // std::cout << "collision: " << w.second->getId() << std::endl;
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
  state = PlayerState::jumping;
  if(checkForCollisions()){
    for(Wall* w : collisions){
      if(collisionTop(w)) return;
    }
  }
  if ( player.getY() > 0 && energy > 0) {
    if(player.getVelocityY() > 0) player.setVelocityY(0);
    player.setVelocityY( player.getVelocityY() - flyPower );
    energy -= flyPower;
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

void Player::handleGravity(){
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
      // std::cout << "falling" << std::endl;
    }
    if(collisionTop(w)){
      player.setVelocityY(0);
    }
    if(collisionRight(w) || collisionLeft(w)){
      player.setVelocityX(0);
    }
  }
  // }
  if(state != PlayerState::idle){
    player.setVelocityY(player.getVelocityY() + gravity);
  } else {
    refillEnergy();
    stop();
  }
}

void Player::refillEnergy(){
  energy = LevelManager::UNIT_SIZE*4*totalEnergies;
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

  handleGravity();
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
