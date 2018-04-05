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
  void attach(SmartSprite* o) { observers.push_back(o); }
  void detach(SmartSprite* o);

  const std::string& getName() const { return player.getName(); }
  int getX() const { return player.getX(); }
  int getY() const { return player.getY(); }
  const Image* getImage() const { return player.getImage(); }
  int getScaledWidth()  const { return player.getScaledWidth(); }
  int getScaledHeight()  const { return player.getScaledHeight(); }
  const SDL_Surface* getSurface() const { return player.getSurface(); }

  void addLight(Light* l) { lights.push_back(l); }
  const std::vector<Light*>& getLights() { return lights; }

  const Vector2f& getPosition() const { return player.getPosition(); }
  void setPosition(const Vector2f& v) { player.setPosition(v); }

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

private:
  MultiSprite player;
  Vector2f initialVelocity;
  std::list<SmartSprite*> observers;
  std::vector<Light*> lights; // can remove this and only have one light pointer
  std::list<Wall*> collisions;
  std::vector<Collectable*> collectables;
  PlayerState state;
  int worldWidth;
  int worldHeight;
  bool updateLighting;
  int hoverHeight;
  int energy;
  int flyPower;
  int totalEnergies;

  bool checkWallCollision(Wall* w);
  bool checkForCollisions();
  bool collisionRight(Wall* w);
  bool collisionLeft(Wall* w);
  bool collisionTop(Wall* w);
  bool collisionBottom(Wall* w);
  void updatePlayerState();
  void handleGravity();
  void refillEnergy();
  int  useEnergy(int i);
  int  maxEnergy();

  Player& operator=(const Player&) = delete;
};
#endif
