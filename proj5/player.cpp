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
  totalEnergies(1),
  renderCollectableLight(Gamedata::getInstance().getXmlBool("Player/collectableLight")),
  alive(true)
  {
    // checkForCollisions();
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
  if(noClip == false){
    for(Wall* w : collisions){
      if(LevelManager::getInstance().getStrategy()->collisionRight(&player, w))
        return;
    }
  }
  if ( player.getX() < worldWidth-getScaledWidth()) {
    player.setVelocityX(initialVelocity[0]);
  }
  updateLighting = true;
}

void Player::left()  {
  state = PlayerState::walking;
  if(noClip == false){
    for(Wall* w : collisions){
      if(LevelManager::getInstance().getStrategy()->collisionLeft(&player, w))
        return;
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
  if(noClip){
    if(player.getY() > 0){
      player.setVelocityY(-initialVelocity[1]);
    } else {
      player.setVelocityY(initialVelocity[1]);
    }
  } else {
    if(noClip == false){
      for(Wall* w : collisions){
        if(LevelManager::getInstance().getStrategy()->collisionTop(&player, w)){
          // always use energy if pressing up
          if(useEnergy((int)ticks)) player.setVelocityY(0);
          return;
        }
      }
    }
    if(player.getY() > 0 && useEnergy((int)ticks)){
      player.setVelocityY( -(int)flyPower);
    } else if(player.getVelocityY() > flyPower){
      player.setVelocityY(flyPower); // allow player to fall slowly
    }
  }
}

// only used when player is in noClip mode
void Player::down()  {
  if(noClip == false) return;
  if ( player.getY() < worldHeight-getScaledHeight()) {
    player.setVelocityY( initialVelocity[1] );
  }
  updateLighting = true;
}

void Player::updatePlayerState(){
  updateLighting = true;
  if(noClip == true) {
    stop();
    return;
  }
  if(player.getVelocityY() > 0){
    state = PlayerState::falling;
  } else if(player.getVelocityY() < 0){
    state = PlayerState::jumping;
  } else if(player.getVelocityX() != 0){
    state = PlayerState::walking;
    player.setVelocityX(0);  // done to stop player from skating around
  } else {
    state = PlayerState::idle;
    updateLighting = false;
  }
}

void Player::handleGravity(Uint8 ticks){
  if(noClip == true) {
    energy = maxEnergy();
    return;
  }
  float gravity = 10; // cool constant you have there..
  float terminalVelocity = 400;

  // check if they should be falling
  checkForCollisions();
  bool falling = true;
  for(Wall* w : collisions){
    if(collisionBottom(w)){
      falling = false;
      refillEnergy(ticks);
    }
    if(LevelManager::getInstance().getStrategy()->collisionTop(&player, w)){
      player.setVelocityY(0);
    }
    if(LevelManager::getInstance().getStrategy()->collisionRight(&player, w) || LevelManager::getInstance().getStrategy()->collisionLeft(&player, w)){
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

void Player::refillEnergy(Uint8 ticks){
  if(player.getVelocityY() == 0 && energy < maxEnergy()){
    energy += (int)ticks*5;
    light->setIntensity(calculateLightIntensity(light));
  }
  if(energy > maxEnergy()) energy = maxEnergy();
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
  player.setPosition(v + Vector2f(-getScaledWidth()/2,
                                  -(hoverHeight + getScaledHeight()/2)));
  updateLight();
  alive = true;
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
  if(renderCollectableLight){
    return l->getBaseIntensity()*(1+(4.0*energy/maxEnergy()))/5.0;
  } else {
    return l->getBaseIntensity()*
           (1+(4.0*energy/maxEnergy()))/5.0*(collectables.size()+1);
  }
}

void Player::addCollectable(Collectable* c) {
  collectables.push_back(c);
}

void Player::damagePlayer(){
  if((int)collectables.size() > 0){
    collectables.back()->explode();
    collectables.pop_back();
  } else {
    // PLAYER IS DEAD!!!
    killPlayer();
  }
}

void Player::killPlayer(){
  player.explode();
  alive = false;
}

void Player::updateCollectables(){
  for(int i = 0; i < (int)collectables.size(); i++){
    float angle = 3.14 * ((float)i/collectables.size())*2; // fixed position
    if(collectables[i]->getRotate()){
      angle += Clock::getInstance().getTicks()*0.001 ; // rotating
    }
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
    if(renderCollectableLight){
      c->setLightIntensity(calculateLightIntensity(c->getLight()));
    } else if(c->getLight()->getRenderStatus()){
      c->getLight()->setRenderStatus(false);
    }
  }
}

void Player::update(Uint32 ticks) {
  if(isDead() && player.isExploding() == false ) return;
  player.update(ticks);
  totalEnergies = (int)collectables.size() + 1;

  handleGravity(ticks);
  updatePlayerState();

  if(updateLighting){
    updateLight();
  }
  updateCollectables();
}

void Player::draw() const {
  if(isDead() && player.isExploding() == false ) return;

  player.draw();
}
