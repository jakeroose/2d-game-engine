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
#include "collisionStrategy.h"
#include "viewport.h"
#include "levelManager.h"
#include "collectable.h"
#include "lightRenderer.h"
#include "background.h"

Engine::~Engine() {
  delete player;
  delete strategy;
  std::cout << "Terminating program" << std::endl;
}

Engine::Engine() :
  rc( RenderContext::getInstance() ),
  clock( Clock::getInstance() ),
  renderer( rc->getRenderer() ),
  viewport( Viewport::getInstance() ),
  hud(HUD("hud")),
  pauseMenu(HUD( "pauseMenu",
    Gamedata::getInstance().getXmlInt("pauseMenu/width"),
    Gamedata::getInstance().getXmlInt("pauseMenu/height"),
    Gamedata::getInstance().getXmlInt("view/width")/2,
    Gamedata::getInstance().getXmlInt("view/height")/2)),
  player(new Player("Player")),
  strategy(),
  background(
    new Background(Gamedata::getInstance().getXmlInt("background/count"))
  ),
  currentStrategy(0),
  currentSprite(0),
  collision(false),
  makeVideo( false )
{
  hud.toggleDisplay();

  player->respawn(LevelManager::getInstance().getSpawnPoint());

  strategy = new RectangularCollisionStrategy;

  // initialize background
  background->initialize();

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
  LightRenderer::getInstance().draw();

  player->draw();
  for(Collectable* c: LevelManager::getInstance().getCollectables()){
    c->draw();
  }
  for(auto e: LevelManager::getInstance().getEnemies()) e->draw();

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

  if(player->isDead()){
    SDL_Color white = {255, 255, 255, 255};
    IoMod::getInstance().writeText(" You Died!", Viewport::getInstance().getViewWidth()/2 - 50, Viewport::getInstance().getViewHeight()/2 - 30, white);
    IoMod::getInstance().writeText("R to restart", Viewport::getInstance().getViewWidth()/2 - 50, Viewport::getInstance().getViewHeight()/2 + 20, white);
  }

  if(hud.getDisplay()){
    IoMod::getInstance().writeText("Debug Info", 15, 15);
    // strategy->draw();

    // write FPS to screen
    std::stringstream strm;
    // strm.str(std::string()); // clear strm
    strm << Clock::getInstance().getFps();
    IoMod::getInstance().writeText("FPS: " + strm.str(), 15, 50);
    IoMod::getInstance().writeText("PlayerState: "+player->getStateStr(), 15, 75);
    IoMod::getInstance().writeText("PlayerEngergy: " +
      std::to_string(player->getEnergy()), 15, 100);
    IoMod::getInstance().writeText("Walls:             " +      std::to_string(LevelManager::getInstance().getWallCount()), 15, 125);
    IoMod::getInstance().writeText("Free Walls:        " +      std::to_string(LevelManager::getInstance().getFreeWallCount()), 15, 150);
    IoMod::getInstance().writeText("Collectables:      " +      std::to_string(LevelManager::getInstance().getCollectableCount()), 15, 175);
    IoMod::getInstance().writeText("Free Collectables: " +      std::to_string(LevelManager::getInstance().getFreeCollectableCount()), 15, 200);
    int totalIntersections = LevelManager::getInstance().getTotalLightIntersections() + player->getLight()->getPolygonSize();
    int freeIntersections = LevelManager::getInstance().getTotalFreeIntersections() + player->getLight()->getIntersectionPoolSize();
    IoMod::getInstance().writeText("Light Intersections: " +      std::to_string(totalIntersections), 15, 225);
    IoMod::getInstance().writeText("Free Intersections : " +      std::to_string(freeIntersections), 15, 250);
    if(LevelManager::getInstance().inEditMode()){
      Vector2f v =LevelManager::getInstance().getCursor();
      IoMod::getInstance().writeText((std::to_string((int)v[0]) + " " + std::to_string((int)v[1])), 15, 275);
    }

    // IoMod::getInstance().writeText("PlayerEngergy: " +
    //   std::to_string(player->getTotalEnergies()), 15, 125);

    // strm.str(std::string()); // clear strm
    // strm << clock.getElapsedTicks();
    // IoMod::getInstance().writeText("Ticks: " + strm.str(), 15, 125);

    // IoMod::getInstance().writeText("Controls:", 15, 75);
    // IoMod::getInstance().writeText("M   - Collision Strat.", 15, 100);
    // IoMod::getInstance().writeText("F1  - Toggle HUD", 15, 125);
    // IoMod::getInstance().writeText("P   - Pause", 15, 150);
    // IoMod::getInstance().writeText("ESC - Quit", 15, 175);
    // IoMod::getInstance().writeText("Click - Add Sprite", 15, 200);
    //
    // // draw movement info to HUD
    // IoMod::getInstance().writeText("WASD to move", 15, 250);
    // const Uint8* keystate;
    // keystate = SDL_GetKeyboardState(NULL);
    // if (keystate[SDL_SCANCODE_A]) {
    //   IoMod::getInstance().writeText("left", 15, 300);
    // }
    // if (keystate[SDL_SCANCODE_D]) {
    //   IoMod::getInstance().writeText("right", 65, 300);
    // }
    // if (keystate[SDL_SCANCODE_W]) {
    //   IoMod::getInstance().writeText("up", 40, 275);
    // }
    // if (keystate[SDL_SCANCODE_S]) {
    //   IoMod::getInstance().writeText("down", 25, 325);
    // }
  }

  // draw our menus
  hud.draw();
  pauseMenu.draw();




  // display my name extravagantly
  Uint8 rndm = 128.0*sin(Clock::getInstance().getTicks()*0.001)+127;
  SDL_Color custColor = {128,rndm,128,255};
  IoMod::getInstance().writeText("Jake Roose", 10 + (rndm/255.0)*20, Viewport::getInstance().getViewHeight() - 30, custColor);

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
    // c->setPosition(c->getPosition()*1.000001);
  }
}

void Engine::update(Uint32 ticks) {
  checkForCollisions();
  // LevelManager::getInstance().updateViewBorder();
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

  background->update(ticks);
  // world.update();
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
  pauseMenu.toggleDisplay();

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
          if ( clock.isPaused() ) clock.unpause();
          else clock.pause();
          pauseMenu.toggleDisplay();
          pauseMenu.drawCentered();
        }
        if ( keystate[SDL_SCANCODE_T] ) {
          switchSprite();
        }
        if ( keystate[SDL_SCANCODE_R] ) {
          LevelManager::getInstance().loadLevel("levels/" +
                          Gamedata::getInstance().getXmlStr("level/name"));
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
