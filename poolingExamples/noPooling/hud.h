#ifndef HUD__H
#define HUD__H
#include <SDL2/SDL.h>
#include <string>

class HUD {
public:
  HUD(std::string s, int w, int h, int _x, int _y);
  HUD(std::string s);
  HUD(const HUD& h) : name(h.name), width(h.width), height(h.height), x(h.x), y(h.y), renderer(h.renderer), display(h.display) {}
  void draw() const;
  void drawCentered() const;
  bool getDisplay() const { return display; }
  void toggleDisplay() { display = !display; }
  const HUD& operator=(const HUD& h) = delete;
  void getMenuItems();
private:
  std::string name;
  int width;
  int height;
  // where to draw in viewport
  int x;
  int y;
  SDL_Renderer* renderer;
  bool display;  // should you draw the HUD
};

#endif
