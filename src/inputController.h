#include <array>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
struct MouseState {
  float x=0, y=0;
  std::array<bool, 5> down{};     // left=0, middle=1, right=2, etc.
  std::array<bool, 5> pressed{};
  std::array<bool, 5> released{};
};

class InputController {
  std::array<bool, SDL_SCANCODE_COUNT> down{}, pressed{}, released{};
  MouseState mouse;

  bool firstMouse = true;

public:
  void BeginFrame() {
    pressed.fill(false);
    released.fill(false);
    mouse.pressed.fill(false);
    mouse.released.fill(false);
  }

  void ProcessEvent(const SDL_Event &e) {
    switch (e.type) {
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP: {
        SDL_Scancode sc = e.key.scancode;
        bool isDown = (e.type == SDL_EVENT_KEY_DOWN);
        if (isDown != down[sc]) {
          if (isDown) pressed[sc] = true;
          else released[sc] = true;
          down[sc] = isDown;
        }
      } break;
// float xpos = static_cast<float>(event.motion.x);
            //     float ypos = static_cast<float>(event.motion.y);
      case SDL_EVENT_MOUSE_MOTION:
        mouse.x = static_cast<float>(e.motion.x);
        mouse.y = static_cast<float>(e.motion.y);
        break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      case SDL_EVENT_MOUSE_BUTTON_UP: {
        int idx = e.button.button - 1;
        bool isDown = (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
        if (idx >= 0 && idx < (int)mouse.down.size() && isDown != mouse.down[idx]) {
          mouse.down[idx] = isDown;
          if (isDown) mouse.pressed[idx] = true;
          else mouse.released[idx] = true;
        }
      } break;

      default: break;
    }
  }

  bool IsKeyDown(SDL_Scancode sc) const { return down[sc]; }
  bool WasKeyPressed(SDL_Scancode sc) const { return pressed[sc]; }
  bool WasKeyReleased(SDL_Scancode sc) const { return released[sc]; }

  float GetMouseX() const { return mouse.x; }
  float GetMouseY() const { return mouse.y; }
  bool IsMouseDown(int btn=0) const { return mouse.down[btn]; }
  bool WasMousePressed(int btn=0) const { return mouse.pressed[btn]; }
  bool WasMouseReleased(int btn=0) const { return mouse.released[btn]; }
};
