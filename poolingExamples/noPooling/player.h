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
  const Image* getImage() const {
    return player.getImage();
  }
  int getScaledWidth()  const {
    return player.getScaledWidth();
  }
  int getScaledHeight()  const {
    return player.getScaledHeight();
  }
  const SDL_Surface* getSurface() const {
    return player.getSurface();
  }

  void addLight(Light* l) { lights.push_back(l); }
  const std::vector<Light*>& getLights() { return lights; }

  const Vector2f& getPosition() const { return player.getPosition(); }
  void setPosition(const Vector2f& v) { player.setPosition(v); }

  void right();
  void left();
  void up();
  void down();
  void stop();


private:
  MultiSprite player;
  Vector2f initialVelocity;
  std::list<SmartSprite*> observers;
  std::vector<Light*> lights;
  int worldWidth;
  int worldHeight;
  bool updateLighting;

  Player& operator=(const Player&) = delete;
};
#endif
