#ifndef PLAYER__H
#define PLAYER__H
#include <SDL.h>
#include <string>
#include <vector>
#include <cmath>
#include <list>
#include "multisprite.h"
#include "smartSprite.h"
#include "light.h"
#include "collectable.h"

class Collectable;

/* === Player States ===
idle: they are standing on the floor. cannot enter another state w/out input
falling: yVelocity > 0
jumping: yVelocity < 0
walking: yVelocity = 0 && xVelocity != 0
*/
enum class PlayerState { idle, falling, jumping, walking };

class Player {
public:
  Player(const std::string&);
  Player(const Player&);
  ~Player();

  void draw() const;
  void update(Uint32 ticks);
  const MultiSprite* getPlayer() const { return &player; }

  const std::string& getName() const { return player.getName(); }
  int getX() const { return player.getX(); }
  int getY() const { return player.getY(); }
  const Image* getImage() const { return player.getImage(); }
  int getScaledWidth()  const { return player.getScaledWidth(); }
  int getScaledHeight()  const { return player.getScaledHeight(); }
  const SDL_Surface* getSurface() const { return player.getSurface(); }
  Light* getLight() const { return light; }

  const Vector2f& getPosition() const { return player.getPosition(); }
  void setPosition(const Vector2f& v) { player.setPosition(v); }
  void respawn(const Vector2f& v);
  void reset();
  void updateLight();

  void right();
  void left();
  void up();
  void up(Uint32 ticks);
  void down();
  void stop();

  PlayerState getState() const { return state; }
  const std::string getStateStr();
  int getEnergy() const { return energy; }
  void addCollectable(Collectable* c);
  void removeCollectable();
  void removeCollectable(Collectable* c);
  int getTotalEnergies() const { return (int)collectables.size(); }
  void damagePlayer();
  void killPlayer();
  bool isDead() const { return !alive; }
  void toggleNoClip() { noClip = !noClip; }
  void toggleGodMode() { godMode = !godMode; }

private:
  MultiSprite player;
  Vector2f initialVelocity;
  Light* light;
  std::list<Wall*> collisions;
  std::vector<Collectable*> collectables;
  PlayerState state;
  int worldWidth;
  int worldHeight;
  bool updateLighting;
  bool noClip;
  bool godMode;
  int hoverHeight;
  int energy;
  int flyPower;
  int totalEnergies;
  bool renderCollectableLight;
  bool alive;

  bool checkWallCollision(Wall* w);
  bool checkForCollisions();
  bool collisionBottom(Wall* w);
  void updatePlayerState();
  void handleGravity(Uint8 t);
  void refillEnergy(Uint8 t);
  void updateCollectables();
  int  useEnergy(int i);
  int  maxEnergy();
  int calculateLightIntensity(Light* l);

  Player& operator=(const Player&) = delete;
};
#endif
