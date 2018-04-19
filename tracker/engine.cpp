#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <random>
#include <iomanip>
#include "sprite.h"
#include "smartSprite.h"
#include "multisprite.h"
#include "ioMod.h"
#include "twowaymultisprite.h"
#include "gamedata.h"
#include "engine.h"
#include "frameGenerator.h"
#include "player.h"
#include "levelManager.h"
#include "collectable.h"
#include "lightRenderer.h"

Engine::~Engine() {
  delete player;
  delete strategy;
  std::cout << "Terminating program" << std::endl;
  delete background;
}

Engine::Engine() :
  rc( RenderContext::getInstance() ),
  clock( Clock::getInstance() ),
  renderer( rc->getRenderer() ),
  viewport( Viewport::getInstance() ),
  hud(HUDManager::getInstance()),
  player(new Player("Player")),
  strategy(new RectangularCollisionStrategy()),
  background(
    new Background(Gamedata::getInstance().getXmlInt("background/count"))
  ),
  currentStrategy(0),
  currentSprite(0),
  collision(false),
  makeVideo( false ),
  displayName(Gamedata::getInstance().getXmlBool("displayName"))
{
  // hud.toggleDisplay();
  player->respawn(LevelManager::getInstance().getSpawnPoint());

  // initialize background
  background->initialize();

  hud.initialize(player);

  Viewport::getInstance().setObjectToTrack(player->getPlayer());
  std::cout << "Loading complete" << std::endl;
}

// add sprite to ~(x,y) in the viewport
void Engine::addSprite(int x, int y){
  std::cout << "add enemy" << std::endl;
  LevelManager::getInstance().addEnemy(
    (x + Viewport::getInstance().getX())/LevelManager::UNIT_SIZE,
    (y + Viewport::getInstance().getY())/LevelManager::UNIT_SIZE);
}

void Engine::draw() const {
  SDL_SetRenderDrawColor( renderer, 15, 15, 15, 255 );
  SDL_RenderClear(renderer);
  background->draw();
  LevelManager::getInstance().getGoal()->draw();
  LightRenderer::getInstance().draw();

  player->draw();
  for(Collectable* c: LevelManager::getInstance().getCollectables()){
    c->draw();
  }

  float x, y, min0, alpha;
  for(auto e: LevelManager::getInstance().getEnemies()){

    /* === ENEMIES FADE BASED ON DISTANCE FROM PLAYER === */
    // TODO: Move this to Drawable
    x = e->getX() - player->getX();
    y = e->getY() - player->getY();
    // set between range of 0 and 255
    min0 = std::max((255*(float)(viewport.getViewWidth()/2 -
      (hypot(x, y)-50))/(float)(viewport.getViewWidth()/2)), 0.0f);
    alpha = std::min(min0, 255.0f);
    SDL_SetTextureAlphaMod(e->getImage()->getTexture(), alpha);

    e->draw();
  }

  // TODO: Move this into LevelEditor class
  if(LevelManager::getInstance().inEditMode()){
    // draw coord grid
    SDL_SetRenderDrawColor(renderer, 0,0,255,255/2);
    int w = viewport.getWorldWidth() / LevelManager::UNIT_SIZE;
    int h = viewport.getWorldHeight() / LevelManager::UNIT_SIZE;
    for(int i = 0; i < w; i++){
      SDL_RenderDrawLine(renderer,
        (int)(i*LevelManager::UNIT_SIZE - viewport.getX()), 0,
        (int)(i*LevelManager::UNIT_SIZE - viewport.getX()), viewport.getViewHeight());
    }
    for(int i = 0; i < h; i++){
      SDL_RenderDrawLine(renderer, 0,
        (int)(i*LevelManager::UNIT_SIZE - viewport.getY()),
        viewport.getViewWidth(),
        (int)(i*LevelManager::UNIT_SIZE - viewport.getY()));
    }

    SDL_SetRenderDrawColor(renderer, 255,0,128,255);
    for(auto w: LevelManager::getInstance().getWalls()) w.second->draw();

    // draw coord at nearest viable coordinate
    Vector2f v = LevelManager::getInstance().getCursor();
    SDL_Rect r = { (int)(v[0] - 5 - viewport.getX()),
      (int)(v[1] - 5 - viewport.getY()), 10, 10};
    SDL_RenderDrawRect(renderer, &r);

    // draw rect in the making from anchor to coord
    if(LevelManager::getInstance().getAnchor()){
      Vector2f anchor = *(LevelManager::getInstance().getAnchor());
      r = {(int)(v[0] - viewport.getX()),
        (int)(v[1] - viewport.getY()),
        (int)(anchor[0]-v[0]),
        (int)(anchor[1]-v[1])
      };
      SDL_RenderDrawRect(renderer, &r);
    }
  }

  hud.draw();

  // display my name extravagantly
  // no longer extravagant due to new HUD system. need to use the string+color
  // the the key in the hudElement map
  // TODO: either fix or just display my name like a regular pleb
  if(displayName){
    Uint8 rndm = 128.0*sin(Clock::getInstance().getTicks()*0.001)+127;
    SDL_Color custColor = {128,rndm,128,255};
    IoMod::getInstance().writeText("Jake Roose", 10 + (rndm/255.0)*20,
      Viewport::getInstance().getViewHeight() - 30, custColor);
  }


  // we don't do anything in Viewport::draw() anymore
  // viewport.draw();
  SDL_RenderPresent(renderer);
}

void Engine::checkForCollisions() {
  for(auto e: LevelManager::getInstance().getEnemies()){
    if((!player->isDead() && !e->isDead()) &&
        strategy->execute(*(player->getPlayer()), *e)){
      e->kill();
      player->damagePlayer();
    }
  }

  for(Collectable* c: LevelManager::getInstance().getCollectables()){
    if ( strategy->execute(*(player->getPlayer()), *(c->getSprite())) ) {
      c->collect(player);
    }
  }

  if(strategy->execute(*(player->getPlayer()),
     *(LevelManager::getInstance().getGoal()))){
    LevelManager::getInstance().setGoalReached(true);
  }
}

void Engine::update(Uint32 ticks) {

  if(LevelManager::getInstance().isPaused() == false){
    checkForCollisions();
    player->update(ticks);
    for(auto e: LevelManager::getInstance().getEnemies()){
      e->update(ticks);
      if(e->isDead() && e->isExploding() == false){
        LevelManager::getInstance().removeEnemy(e);
      }
    }

    for(Collectable* c: LevelManager::getInstance().getCollectables()){
      c->update(ticks);
      if(c->doneExploding()){
        LevelManager::getInstance().removeCollectable(c);
      }
    }

    // TODO: Move all of this to HUDManager and config files
    // would require either variables in the string file that get mapped to
    // a function or pass a function pointer that returns the strings
    if(hud.getDisplay()){
      HUDElement* tmp = HUDManager::getInstance().getElement("hud");
      tmp->clear();
      // write FPS to screen
      std::stringstream strm;
      strm << "FPS:" << Clock::getInstance().getFps();

      int totalIntersections =
      LevelManager::getInstance().getTotalLightIntersections() +
      player->getLight()->getPolygonSize();
      int freeIntersections =
      LevelManager::getInstance().getTotalFreeIntersections() +
      player->getLight()->getIntersectionPoolSize();

      std::vector<std::string> s = {
        "DEBUG INFO",
        strm.str(),
        "PlayerState: "+ player->getStateStr(),
        "PlayerEngergy: " + std::to_string(player->getEnergy()),
        "Walls:             " +
        std::to_string(LevelManager::getInstance().getWallCount()),
        "Free Walls:        " +
        std::to_string(LevelManager::getInstance().getFreeWallCount()),
        "Collectables:      " +
        std::to_string(LevelManager::getInstance().getCollectableCount()),
        "Free Collectables: " +
        std::to_string(LevelManager::getInstance().getFreeCollectableCount()),
        "Light Intersections: " + std::to_string(totalIntersections),
        "Free Intersections : " + std::to_string(freeIntersections)
      };
      if(LevelManager::getInstance().inEditMode()){
        Vector2f v = LevelManager::getInstance().getCursor() /
        LevelManager::UNIT_SIZE;
        s.push_back("X: " + std::to_string((int)v[0]) +
        ", Y: " + std::to_string((int)v[1]));
      }

      tmp->addLines(s);
    }
  }

  background->update(ticks);
  viewport.update(); // always update viewport last
}

// doesn't actually switch
void Engine::switchSprite(){
  Viewport::getInstance().setObjectToTrack(player->getPlayer());
}

void Engine::play() {
  SDL_Event event;
  const Uint8* keystate;
  bool done = false;
  Uint32 ticks = clock.getElapsedTicks();
  FrameGenerator frameGen;
  // hud.toggleDisplay();

  while ( !done ) {
    // The next loop polls for events, guarding against key bounce:
    while ( SDL_PollEvent(&event) ) {
      keystate = SDL_GetKeyboardState(NULL);
      if (event.type ==  SDL_QUIT) { done = true; break; }
      if(event.type == SDL_KEYDOWN) {
        if (keystate[SDL_SCANCODE_ESCAPE] || keystate[SDL_SCANCODE_Q]) {
          done = true;
          break;
        }
        if ( keystate[SDL_SCANCODE_P] ) {
          LevelManager::getInstance().togglePause();
        }
        if ( keystate[SDL_SCANCODE_R] ) {
          LevelManager::getInstance().resetLevel();
          player->reset();
        }
        if ( keystate[SDL_SCANCODE_N] ) {
          player->toggleNoClip();
        }
        if ( keystate[SDL_SCANCODE_F8]) {
          LevelManager::getInstance().toggleLevelEdit();
        }
        if ( keystate[SDL_SCANCODE_F9]) {
          LevelManager::getInstance().saveLevel();
        }
        if (keystate[SDL_SCANCODE_F1]) {
          hud.toggleDisplay();
        }
        if (keystate[SDL_SCANCODE_F2]) {
          LightRenderer::getInstance().toggleDebug();
        }
        if (keystate[SDL_SCANCODE_F4] && !makeVideo) {
          std::cout << "Initiating frame capture" << std::endl;
          makeVideo = true;
        }
        else if (keystate[SDL_SCANCODE_F4] && makeVideo) {
          std::cout << "Terminating frame capture" << std::endl;
          makeVideo = false;
        }
        if(LevelManager::getInstance().getGoalReached() &&
            (keystate[SDL_SCANCODE_RETURN] || keystate[SDL_SCANCODE_KP_ENTER])){
          // TODO: store next level name as a variable in the level file
          LevelManager::getInstance().loadLevel("levels/bigLevel");
          player->reset();
        }
      }

      // handle mouse interaction
      if(event.type == SDL_MOUSEBUTTONDOWN) {
        if(LevelManager::getInstance().inEditMode()){
          if(event.button.button == SDL_BUTTON_RIGHT){
            LevelManager::getInstance().resetAnchor();
          } else {
            LevelManager::getInstance().setAnchor();
          }
          player->getLight()->update();
        } else {
          int x, y;
          SDL_GetMouseState( &x, &y);
          addSprite(x, y);
        }
      }

      if(event.type == SDL_MOUSEMOTION) {
        if(LevelManager::getInstance().inEditMode()){
          // get mouse position
          int x, y;
          SDL_GetMouseState( &x, &y);
          LevelManager::getInstance().setCursor(Vector2f(
            x+Viewport::getInstance().getX(),
            y+Viewport::getInstance().getY())
          );
        }
      }
    }

    // In this section of the event loop we allow key bounce:
    ticks = clock.getElapsedTicks();
    if ( ticks > 0 ) {
      clock.incrFrame();
      if (keystate[SDL_SCANCODE_A]) {
        static_cast<Player*>(player)->left();
      }
      if (keystate[SDL_SCANCODE_D]) {
        static_cast<Player*>(player)->right();
      }
      if (keystate[SDL_SCANCODE_W] || keystate[SDL_SCANCODE_SPACE]) {
        static_cast<Player*>(player)->up(ticks);
      }
      if (keystate[SDL_SCANCODE_S]) {
        static_cast<Player*>(player)->down();
      }
      update(ticks);
      draw();
      if ( makeVideo ) {
        frameGen.makeFrame();
      }
    }
  }
}
