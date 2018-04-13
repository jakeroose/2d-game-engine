#include <vector>
#include <SDL.h>
#include "renderContext.h"
#include "clock.h"
#include "world.h"
#include "viewport.h"
#include "hud.h"
#include "collisionStrategy.h"
#include "background.h"

class Player;
class CollisionStrategy;
class SmartSprite;

enum class EngineState { run, pause, edit };

class Engine {
public:
  Engine ();
  ~Engine ();
  void play();
  void switchSprite();

private:
  const RenderContext* rc;
  Clock& clock;

  SDL_Renderer * const renderer;
  World world;
  Viewport& viewport;
  HUD hud;
  HUD pauseMenu;

  Player* player;
  RectangularCollisionStrategy* strategy;
  Background* background;

  int currentStrategy;
  int currentSprite;
  bool collision;

  bool makeVideo;

  void draw() const;
  void update(Uint32);

  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  void printScales() const;
  void checkForCollisions();
  void addSprite();
  void addSprite(int x, int y);
};
