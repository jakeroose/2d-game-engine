#include <vector>
#include <SDL.h>
#include "renderContext.h"
#include "clock.h"
#include "world.h"
#include "viewport.h"
#include "hudManager.h"
#include "collisionStrategy.h"
#include "background.h"
#include "sound.h"

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
  Viewport& viewport;
  HUDManager& hud;
  Player* player;
  RectangularCollisionStrategy* strategy;
  Background* background;
  SDLSound& sound;

  int currentStrategy;
  int currentSprite;
  bool collision;
  bool makeVideo;
  bool displayName;


  void draw() const;
  void update(Uint32);

  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  void printScales() const;
  void checkForCollisions();
  void addSprite();
  void addSprite(int x, int y);
};
