#include "raylib.h"
#include <math.h>
#include <stdio.h>

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 1080;
  const int screenHeight = 720;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(screenWidth, screenHeight, "pajaros ENOJONES");

  float Xpos = 0.0;                      // Posicion de la bola en X
  float Ypos = (float)GetScreenHeight(); // Posicion de la bola en Y
  float theta = 0;
  float Vi = 0;       // Angulo del lanzamiento
  bool pause = false; // Bandera de pausa

  //--------------------------------------------------------------------------------------

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------

    if (IsKeyPressed(KEY_SPACE))
      pause = !pause;
    if (IsKeyPressed(KEY_UP)) {
      printf("%f\n", theta);
      theta += (PI / 18);
    } else if (IsKeyPressed(KEY_DOWN)) {
      theta -= (PI / 18);
      printf("%f\n", theta);
    }
    if (IsKeyPressed(KEY_RIGHT)) {
      printf("%f\n", Vi);
      Vi += 2.5;
    } else if (IsKeyPressed(KEY_LEFT)) {
      Vi -= 2.5;
      printf("%f\n", Vi);
    }

    if (!pause) {
      Xpos += Vi * cos(theta) * 20 *
              GetFrameTime(); // We move at 200 pixels per second
      Ypos =
          -((tan(theta) * Xpos) - (4.9 * (1 / pow(Vi, 2)) *
                                   (1 / pow(cos(theta), 2)) * pow(Xpos, 2))) +
          GetScreenHeight();
      if (Xpos >= GetScreenWidth())
        Xpos = 0;
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

    for (int i = 0; i < GetScreenWidth() / 100; i++)
      DrawRectangle(200 * i, 0, 1, GetScreenHeight(), SKYBLUE);

    DrawCircle((int)Xpos, (int)Ypos, 10, RED);

    EndDrawing();
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
