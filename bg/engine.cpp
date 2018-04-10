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
  for(auto e: sprites) delete e;
  delete player;
  delete strategy;

  std::cout << "Terminating program" << std::endl;
}

Engine::Engine() :
  rc( RenderContext::getInstance() ),
  clock( Clock::getInstance() ),
  renderer( rc->getRenderer() ),
  world("back", Gamedata::getInstance().getXmlInt("back/factor") ),
  parallax("parallax", Gamedata::getInstance().getXmlInt("parallax/factor") ),
  viewport( Viewport::getInstance() ),
  hud(HUD("hud")),
  pauseMenu(HUD( "pauseMenu",
    Gamedata::getInstance().getXmlInt("pauseMenu/width"),
    Gamedata::getInstance().getXmlInt("pauseMenu/height"),
    Gamedata::getInstance().getXmlInt("view/width")/2,
    Gamedata::getInstance().getXmlInt("view/height")/2)),
  sprites(std::vector<SmartSprite*>()),
  player(new Player("UFO")),
  // strategies(),
  strategy(),
  background(new Background("Collectable")),
  currentStrategy(0),
  currentSprite(0),
  collision(false),
  makeVideo( false )
{
  hud.toggleDisplay();

  player->setPosition(LevelManager::getInstance().getSpawnPoint());

  strategy = new RectangularCollisionStrategy ;

  background->initialize();

  Viewport::getInstance().setObjectToTrack(player->getPlayer());
  std::cout << "Loading complete" << std::endl;
}

// add sprite to default location
void Engine::addSprite(){
  Vector2f pos = player->getPosition();
  int w = player->getScaledWidth();
  int h = player->getScaledHeight();
  SmartSprite* s = new SmartSprite("Bird", pos, w, h);
  sprites.push_back(s);
  player->attach(s);
}

// add sprite to (x,y) in the viewport
void Engine::addSprite(int x, int y){
  Vector2f pos = player->getPosition();
  int w = player->getScaledWidth();
  int h = player->getScaledHeight();
  SmartSprite* s = new SmartSprite("Bird", pos, w, h, x + Viewport::getInstance().getX(), y+Viewport::getInstance().getY());
  sprites.push_back(s);
  player->attach(s);
}



void Engine::draw() const {
  world.draw();
  parallax.draw();

  if(hud.getDisplay()){
    std::stringstream strm;
    strm << sprites.size() << " Sprites Remaining";
    IoMod::getInstance().writeText(strm.str(), 15, 15);
    strategy->draw();

    // write FPS to screen
    strm.str(std::string()); // clear strm
    strm << Clock::getInstance().getFps();
    IoMod::getInstance().writeText("FPS: " + strm.str(), 15, 40);
    IoMod::getInstance().writeText("PlayerState: "+player->getStateStr(), 15, 75);
    IoMod::getInstance().writeText("PlayerEngergy: "+std::to_string(player->getEnergy()), 15, 100);
  }

  // draw our menus
  hud.draw();
  pauseMenu.draw();

  // display my name extravagantly
  Uint8 rndm = 128.0*sin(Clock::getInstance().getTicks()*0.001)+127;
  SDL_Color custColor = {128,rndm,128,255};
  IoMod::getInstance().writeText("Jake Roose", 10 + (rndm/255.0)*20, Viewport::getInstance().getViewHeight() - 30, custColor);


  background->draw();
  // std::vector<Image*> rotImages = std::vector<Image*>();
  // rotImages.push_back(ImageFactory::getInstance().getImage("Collectable"));
  //
  // Image* i = rotImages[0];
  // for(int j = 0; j < 10; j ++){
  //   SDL_Point cent = {i->getWidth()/2, i->getHeight()/2 };
  //   SDL_Rect pos = i->getSurface()->clip_rect;
  //   pos.y += rndm;
  //   pos.x += j * 50;
  //   SDL_RenderCopyEx(
  //     renderer,
  //     i->getTexture(),
  //     NULL,
  //     &pos,
  //     (double) rndm,
  //     &cent,
  //     SDL_FLIP_NONE
  //   );
  // }



  // we don't do anything in Viewport::draw() anymore
  // viewport.draw();
  SDL_RenderPresent(renderer);
}

void Engine::checkForCollisions() {
  auto it = sprites.begin();
  while ( it != sprites.end() ) {
    if ( strategy->execute(*(player->getPlayer()), **it) ) {
      SmartSprite* doa = *it;
      player->detach(doa);
      delete doa;
      it = sprites.erase(it);
      player->removeCollectable();
    }
    else ++it;
  }

  for(Collectable* c: LevelManager::getInstance().getCollectables()){
    if ( strategy->execute(*(player->getPlayer()), *(c->getSprite())) ) {
      c->collect(player);
    }
  }
}

void Engine::update(Uint32 ticks) {
  checkForCollisions();
  player->update(ticks);
  background->update(ticks);
  // for(auto e: sprites) e->update(ticks);

  // for(Collectable* c: LevelManager::getInstance().getCollectables())
  //   c->update();

  world.update();
  parallax.update();
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
        // if ( keystate[SDL_SCANCODE_M] ) {
        //   currentStrategy = (1 + currentStrategy) % strategies.size();
        // }
        if ( keystate[SDL_SCANCODE_T] ) {
          switchSprite();
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
        // if (keystate[SDL_SCANCODE_W]) {
        //   static_cast<Player*>(player)->up();
        // }
      }

      // handle mouse interaction
      if(event.type == SDL_MOUSEBUTTONDOWN) {
        // get mouse position
        int x, y;
        SDL_GetMouseState( &x, &y);
        addSprite(x, y);
      }

      // move player with mouse
      // if(event.type == SDL_MOUSEMOTION) {
      //   // get mouse position
      //   int x, y;
      //   SDL_GetMouseState( &x, &y);
      //   player->setPosition(Vector2f(x, y));
      //   // addSprite(x, y);
      // }
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
