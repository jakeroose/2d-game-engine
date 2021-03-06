#include "lightRenderer.h"
#include "viewport.h"
#include "gamedata.h"
#include "renderContext.h"
#include "levelManager.h"
#include <algorithm>

/* LightRenderer is in charge of drawing all fo the lights on the level.
  Lights are added to lightRenderer in the Light constructor.
  Render all of the lights in one place so that we don't have to loop through
    all off the pixels in the viewport for each light.
*/

LightRenderer::LightRenderer() :
  renderer(RenderContext::getInstance()->getRenderer()),
  lights(),
  debug(Gamedata::getInstance().getXmlBool("lights/debug")),
  renderLights(Gamedata::getInstance().getXmlBool("lights/renderLights")),
  diffusion(Gamedata::getInstance().getXmlBool("lights/diffusion")),
  diffusionRadius(Gamedata::getInstance().getXmlInt("lights/diffusionRadius")),
  baseColor({
    (Uint8)Gamedata::getInstance().getXmlInt("lights/red"),
    (Uint8)Gamedata::getInstance().getXmlInt("lights/green"),
    (Uint8)Gamedata::getInstance().getXmlInt("lights/blue"),
    (Uint8)Gamedata::getInstance().getXmlInt("lights/alpha")
  })
  {
}

LightRenderer& LightRenderer::getInstance(){
  static LightRenderer lr;
  return lr;
}

float LightRenderer::calcIntensity(float d) const{
  if(diffusion == false) return 1.0f;
  d = abs(d);
  // make sure  0 < intensity < 1
  float max1 = std::min(1.0f, ((diffusionRadius - d) / diffusionRadius));
  return std::max(max1, 0.0f);
}

void LightRenderer::draw() const {
  int minx = 999999, miny=999999, maxx = -1, maxy = -1,
      vx = Viewport::getInstance().getX(), // view position
      vy = Viewport::getInstance().getY(),
      vw = Viewport::getInstance().getViewWidth(), // view width/height
      vh = Viewport::getInstance().getViewHeight(),
      cx = vx + vw/2, // center of screen (aka player pos)
      cy = vy + vh/2;

  if(renderLights){
    /* Optimization so that we only DRAW what is in the viewport and within a
       rectangle surrounding the lightPolygon
    */
    for(Light* l : lights){
      if(l->getMinx() < minx){ minx = l->getMinx(); }
      if(l->getMiny() < miny){ miny = l->getMiny(); }
      if(l->getMaxx() > maxx){ maxx = l->getMaxx(); }
      if(l->getMaxy() > maxy){ maxy = l->getMaxy(); }
    }

    // TODO: set min & max based on diffusionRadius if diffusion
    if(minx < vx){ minx = vx;}
    if(miny < vy){ miny = vy;}
    if(maxx > vx + vw){
      maxx = vw+vx;
    }
    if(maxy > vh+vy){
      maxy = vh+vy;
    }

    /* Render the Lighting Polygon */
    // fill algorithm courtesy of http://alienryderflex.com/polygon_fill/
    int intensity, pixelX, pixelY, i, tense,
    IMAGE_RIGHT = maxx, IMAGE_LEFT = minx;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for(Light* l : lights){
      if(l->shouldDraw() == false) continue;
      
      intensity = l->getIntensity();
      SDL_SetRenderDrawColor( renderer, 200, 200, 255, intensity);

      for(auto e : l->getLightPolygonBorder()){
        pixelY = e.first;
        std::vector<int> nodeX = e.second;
        for (i=0; i< (int) nodeX.size(); i+=2) {
          if   (nodeX[i  ]>=IMAGE_RIGHT) break;
          if   (nodeX[i+1]> IMAGE_LEFT ) {
            if (nodeX[i  ]< IMAGE_LEFT ) nodeX[i  ]=IMAGE_LEFT ;
            if (nodeX[i+1]> IMAGE_RIGHT) nodeX[i+1]=IMAGE_RIGHT;
            if(diffusion){
              // if using diffusion we need to draw each pixel
              for (pixelX=nodeX[i]; pixelX<nodeX[i+1]; pixelX++){
                tense = intensity*calcIntensity(hypot(cx - pixelX, cy-pixelY));
                SDL_SetRenderDrawColor( renderer, 200, 200, 200, tense );
                SDL_RenderDrawPoint(renderer, pixelX - vx, pixelY - vy);
              }
            } else {
              // if we aren't using diffusion then we can draw with the line
              // method which is must faster than drawing each pixel
              SDL_RenderDrawLine(renderer, nodeX[i]-vx, pixelY - vy,
                nodeX[i+1] - vx, pixelY - vy);
            }
          }
        }
      }
    }
  }
  // END fill polygon

  /* === Render debug lines & points === */
  if(debug){
    for(Light* l :lights){
      if(l->shouldDraw() == false) continue;
      std::vector<Intersection*> lightPolygon = l->getPolygon();
      Vector2f position = l->getPosition();
      // draw walls
      // NOTE: not sure if drawing walls here makes a lot of sense...
      SDL_SetRenderDrawColor( renderer, 255, 0, 0, 255/2 );
      for(auto w : LevelManager::getInstance().getWalls()){
        w.second->draw();
      }

      SDL_SetRenderDrawColor( renderer, 0, 0, 255, 255 );
      Intersection* lastIntersect = lightPolygon[0];
      for(Intersection* intersect: lightPolygon){
        SDL_SetRenderDrawColor( renderer, 0, 255, 0, 255/2 );

        // Draw ray laser
        SDL_RenderDrawLine(renderer, position[0]-vx, position[1]-vy,
                           intersect->x - vx, intersect->y - vy);

        // Draw Intersection dot
        SDL_Rect r = {(int)intersect->x-2-vx, (int)intersect->y-2-vy, 4, 4};
        SDL_RenderFillRect(renderer, &r);

        // Connect Polygon
        if(intersect == lightPolygon[0]) continue;
        SDL_RenderDrawLine(renderer, lastIntersect->x - vx, lastIntersect->y-vy,
                           intersect->x-vx, intersect->y-vy);
        lastIntersect = intersect;
      }
    }
  }
}
