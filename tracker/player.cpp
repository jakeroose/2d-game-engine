#include "player.h"
#include "gamedata.h"
#include "smartSprite.h"
#include "viewport.h"
#include "light.h"
#include "levelManager.h"
#include "wall.h"
#include "collectable.h"
#include "clock.h"
#include <algorithm>

/* TODO:
  Use collision detection in RectangularCollisionStrategy instead of here.
*/


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
  light(new Light(getPosition())),
  collisions(),
  collectables(std::vector<Collectable*>()),
  state(PlayerState::falling),
  worldWidth(Gamedata::getInstance().getXmlInt("world/width")),
  worldHeight(Gamedata::getInstance().getXmlInt("world/height")),
  updateLighting(true),
  noClip(Gamedata::getInstance().getXmlBool(name+"/noClip")),
  hoverHeight(LevelManager::UNIT_SIZE/4),
  energy(1),
  flyPower(LevelManager::UNIT_SIZE),
  totalEnergies(1)
  {
    checkForCollisions();
}

Player::~Player(){
  delete light;
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
  if(noClip == false && checkForCollisions()){
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
  if(noClip == false && checkForCollisions()){
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
  if(noClip == false && checkForCollisions()){
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
  if(noClip == false && checkForCollisions()){
    for(Wall* w : collisions){
      if(collisionBottom(w)) return;
    }
  }
  if ( player.getY() < worldHeight-getScaledHeight()) {
    player.setVelocityY( flyPower );
  }
  updateLighting = true;
}

void Player::updatePlayerState(){
  updateLighting = true;
  if(noClip == true) {
    stop();
  }
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
  if(noClip == true) {
    energy = maxEnergy();
    return;
  }
  float gravity = 10; // cool constant you have there..
  float terminalVelocity = 400;

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
    else if (player.getVelocityY() > terminalVelocity)
      player.setVelocityY(terminalVelocity); // prevent falling at mach 10
    // player shouldn't float from left to right when falling with style
    player.setVelocityX(0);
  } else {
    player.setVelocityY(0);
  }
}

void Player::refillEnergy(){
  energy = maxEnergy();
  light->setIntensity(light->getBaseIntensity());
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

void Player::respawn(const Vector2f& v){
  player.setPosition(v + Vector2f(0, -hoverHeight));
  updateLight();
}

void Player::reset(){
  auto it = collectables.begin();
  while(it != collectables.end()) it = collectables.erase(it);
  respawn(LevelManager::getInstance().getSpawnPoint());
}

void Player::updateLight(){
  if(light->shouldDraw()){
    light->setPosition(Vector2f(
      getPosition()[0] + getScaledWidth()/2,
      getPosition()[1] + getScaledHeight()/2
    ));
    light->setIntensity(calculateLightIntensity(light));
  }
  updateLighting = false;
}

// scale light based on energy left, keep a minimum of 1/5 intensity
int Player::calculateLightIntensity(Light* l){
  return l->getBaseIntensity()*(1+(4.0*energy/maxEnergy()))/5.0;
}

void Player::addCollectable(Collectable* c) {
  collectables.push_back(c);
  // totalEnergies += 1;
}

// void Player::removeCollectable(){
//   if((int)collectables.size() > 0){
//     LevelManager::getInstance().removeCollectable(collectables.back());
//     collectables.pop_back();
//     --totalEnergies;
//   }
// }

void Player::damagePlayer(){
  if((int)collectables.size() > 0){
    collectables.back()->explode();
    collectables.pop_back();
  } else {
    // TODO: PLAYER IS DEAD!!!
  }
}



// void Player::removeCollectable(Collectable* c){
//   auto it = std::find_if(collectables.begin(), collectables.end(),
//               [&c](Collectable* e){return e == c;});
//   if(it != collectables.end()){
//     collectables.erase(it);
//     // --totalEnergies;
//     std::cout << "removed energy" << std::endl;
//   }
// }

void Player::updateCollectables(){
  for(int i = 0; i < (int)collectables.size(); i++){
    float angle = Clock::getInstance().getTicks()*0.001 + i*10;
    Collectable* c = collectables[i];
    collectables[i]->setPosition(Vector2f(cos(angle), sin(angle)) * 25 +
      // offset according to player center and collectable center
      (getPosition() + Vector2f(
        getScaledWidth()/2 - c->getSprite()->getScaledWidth()/2,
        getScaledHeight()/2 - c->getSprite()->getScaledHeight()/2)));

    // make sure collectable doesn't go outside of the game
    if(c->getPosition()[0] < 0) c->setPosition(Vector2f(0,c->getPosition()[1]));
    if(c->getPosition()[1] < 0) c->setPosition(Vector2f(c->getPosition()[0],0));
    // check for rightside and bottom of map too, eventually walls

    // update lights on collectable
    c->setLightIntensity(calculateLightIntensity(c->getLight()));
  }
}

void Player::update(Uint32 ticks) {
  player.update(ticks);
  std::list<SmartSprite*>::iterator ptr = observers.begin();
  while ( ptr != observers.end() ) {
    (*ptr)->setPlayerPos( getPosition() );
    ++ptr;
  }
  totalEnergies = (int)collectables.size() + 1;

  handleGravity();
  updatePlayerState();

  if(updateLighting){
    updateLight();
  }

  updateCollectables();
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
