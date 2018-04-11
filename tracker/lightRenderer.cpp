#include "lightRenderer.h"
#include "viewport.h"
#include "gamedata.h"
#include "renderContext.h"
#include "levelManager.h"

/* LightRenderer is in charge of drawing all fo the lights on the level.
  Lights are added to lightRenderer in the Light constructor.
  Render all of the lights in one place so that we don't have to loop through
    all off the pixels in the viewport for each light.
*/

LightRenderer::LightRenderer() :
  renderer(RenderContext::getInstance()->getRenderer()),
  lights(),
  debug(Gamedata::getInstance().getXmlBool("lights/debug")),
  renderLights(Gamedata::getInstance().getXmlBool("lights/renderLights"))
  {

}

LightRenderer& LightRenderer::getInstance(){
  static LightRenderer lr;
  return lr;
}

void LightRenderer::draw() const {
  int minx = 999999, miny=999999, maxx = -1, maxy = -1,
      vx = Viewport::getInstance().getX(),
      vy = Viewport::getInstance().getY();

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
  if(minx < vx){ minx = vx;}
  if(miny < vy){ miny = vy;}
  if(maxx > vx + Viewport::getInstance().getViewWidth()){
    maxx = Viewport::getInstance().getViewWidth()+vx;
  }
  if(maxy > Viewport::getInstance().getViewHeight()+vy){
    maxy = Viewport::getInstance().getViewHeight()+vy;
  }

  /* Render the Lighting Polygon */
  // fill algorithm courtesy of http://alienryderflex.com/polygon_fill/
  int polyCorners, intensity;
  int nodes, pixelX, pixelY, i, j, swap, IMAGE_RIGHT = maxx, IMAGE_LEFT = minx;
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor( renderer, 200, 200, 200, 255/2 );

    //  Loop through the rows of the image.
    for (pixelY=miny; pixelY<maxy; pixelY++) {

      for(Light* l : lights){
        if(l->shouldDraw() == false) continue;

        polyCorners = l->getPolygonSize();
        intensity = l->getIntensity();
        SDL_SetRenderDrawColor( renderer, 200, 200, 200, intensity );

        std::vector<Intersection*> lightPolygon = l->getPolygon();
        std::vector<int> nodeX; nodeX.reserve(1024);
        //  Build a list of nodes.
        nodes=0; j=polyCorners-1;
        for (i=0; i<polyCorners; i++) {
          if ((lightPolygon[i]->y <  (double) pixelY &&
          lightPolygon[j]->y >= (double) pixelY) ||
          (lightPolygon[j]->y <  (double) pixelY &&
          lightPolygon[i]->y >= (double) pixelY)) {
            nodeX[nodes++] = (int)(lightPolygon[i]->x+(pixelY-lightPolygon[i]->y) /
            (lightPolygon[j]->y-lightPolygon[i]->y) *
            (lightPolygon[j]->x-lightPolygon[i]->x));
          }
          j=i;
        }

        //  Sort the nodes, via a simple “Bubble” sort.
        i=0;
        while (i<nodes-1) {
          if (nodeX[i]>nodeX[i+1]) {
            swap=nodeX[i];
            nodeX[i]=nodeX[i+1];
            nodeX[i+1]=swap;
            if (i) i--;
          }
          else { i++; }
        }

        //  Fill the pixels between node pairs.
        for (i=0; i<nodes; i+=2) {
          if   (nodeX[i  ]>=IMAGE_RIGHT) break;
          if   (nodeX[i+1]> IMAGE_LEFT ) {
            if (nodeX[i  ]< IMAGE_LEFT ) nodeX[i  ]=IMAGE_LEFT ;
            if (nodeX[i+1]> IMAGE_RIGHT) nodeX[i+1]=IMAGE_RIGHT;
            for (pixelX=nodeX[i]; pixelX<nodeX[i+1]; pixelX++){
              // have to render lighting to the viewport (vx & vy)
              SDL_RenderDrawPoint(renderer, pixelX - vx, pixelY - vy);
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

      // draw outline of lightPolygon
      // NOTE: Move to ^ if we want to keep re-enable
      // int j = lightPolygon.size()-1;
      // for(int i = 0; i < (int)lightPolygon.size(); i++){
      //   SDL_SetRenderDrawColor( renderer, 0, i*10 % 256, 255, 255 );
      //   SDL_RenderDrawLine(renderer, lightPolygon[i]->x - vx,
      //      lightPolygon[i]->y-vy, lightPolygon[j]->x-vx, lightPolygon[j]->y-vy);
      //   j=i;
      // }
    }

  }

  SDL_RenderPresent(renderer);

}
