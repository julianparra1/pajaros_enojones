#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define MAX_SKY_RECTANGLES 10
#define SKY_RECTANGLE_WIDTH 200

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 800;
  const int screenHeight = 450;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(screenWidth, screenHeight, "PAJAROS ENOJONES");

  float Xpos = 0.0;                      // Posicion de la bola en X
  float Ypos = (float)GetScreenHeight(); // Posicion de la bola en Y

  float Vi = 50; // Velocidad inicial
  //
  const float deltaTheta = PI / 180;          // 10 deg
  float theta = PI / 4;                       // Angulo de tiro
  float sec2Theta = (1 / pow(cos(theta), 2)); // sec(theta)^2

  float reciprocoVi2 = (1 / pow(Vi, 2)) * 4.9; // (1/vi^2) * 4.9
  float tanTheta = tan(theta);                 // Tan(theta)
  float cosTheta = cos(theta);                 // cos(theta)

  bool pause = false; // Bandera de pausa

  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------

    if (IsKeyDown(KEY_SPACE))
      pause = !pause;
    if (IsKeyDown(KEY_UP)) {
      theta += deltaTheta;
      sec2Theta = (1 / pow(cos(theta), 2));
      tanTheta = tan(theta);
      cosTheta = cos(theta);
      printf("Theta: %f\n", theta);
    } else if (IsKeyDown(KEY_DOWN)) {
      theta -= deltaTheta;
      sec2Theta = (1 / pow(cos(theta), 2));
      tanTheta = tan(theta);
      cosTheta = cos(theta);
      printf("Theta: %f\n", theta);
    }
    if (IsKeyDown(KEY_RIGHT)) {
      printf("V: %f\n", Vi);
      Vi += 2.5;
      reciprocoVi2 = (1 / pow(Vi, 2)) * 4.9;
    } else if (IsKeyDown(KEY_LEFT)) {
      Vi -= 2.5;
      printf("V: %f\n", Vi);
      reciprocoVi2 = (1 / powf(Vi, 2)) * 4.9;
    }

    if (!pause) {

      Xpos += Vi * cosTheta * 20 * GetFrameTime();
      Ypos = -((tanTheta * Xpos) - (pow(Xpos, 2) * reciprocoVi2 * sec2Theta)) +
             (float)GetScreenHeight();

      if (Xpos > GetScreenWidth()) {
        Xpos = 0;
      }
      if (Ypos > GetScreenHeight()) {
        Ypos = GetScreenHeight();
        Xpos = 0;
      }
    }

    //----------------------------------------------------------------------------------
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawCircle((int)Xpos, (int)Ypos, 10, RED);

    EndDrawing();
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
