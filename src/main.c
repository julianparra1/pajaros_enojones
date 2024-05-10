#include "raylib.h"
#include "raymath.h"
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

  float Vy = 0;
  float Vx = 0;
  float Vi = 50; // Velocidad inicial

  float theta = PI / 4; // Angulo de tiro

  bool pause = false;      // Bandera de pausa
  bool disparando = false; // DISAPROOOOOOOO

  Vector2 posInicial;
  Vector2 posFinal;
  Vector2 deltaPos = {0, 0};
  //--------------------------------------------------------------------------------------
  // Juego
  while (!WindowShouldClose()) {
    // Update
    //----------------------------------------------------------------------------------

    if (IsKeyDown(KEY_SPACE))
      pause = !pause;

    if (!pause) {
      Xpos += Vx; // y = y_0 + dt*Vy
      Ypos += Vy; // x = x_0 + dt*Vx_0
    }

    Vy += 9.8 * GetFrameTime(); // + y''*dt para Vy

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !disparando) {
      posInicial = GetMousePosition();
      disparando = true;
    } else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      posFinal = GetMousePosition();
      deltaPos = Vector2Subtract(posInicial, posFinal);
      Xpos = posInicial.x;
      Ypos = posInicial.y;

      theta = -atan2f(deltaPos.y, deltaPos.x);

      Vi = sqrt(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y);

      if (Vi > 70) {
        Vi = 70;
      }
      ///////////////////////////////////////////////////////////////////////
      // metodo de EULER oilar !!!!!!!!!!!!!!!!!!!!!!!
      // (REF: https://www.physics.umd.edu/hep/drew/numerical_integration.html)
      //
      // el movimiento de proyectiles es descrito por increibles EDOs...
      //
      // y'(0) = V_y0 = v_0sin(theta)
      // x'(0) = V_x0 = v_0cos(theta)
      //
      // x' = Vx = V_x0; CONST
      // y' = Vy = V_x0 - g*dt (y'' = a_y = -g)
      ///////////////////////////////////////////////////////////////////////
      Vx = Vi * cos(theta) * 15 * GetFrameTime();
      Vy = (-sin(theta) * Vi * 15 * GetFrameTime()); // Invertimos el signo pues el eje y esta invertido

      printf("%f\n", theta);
      printf("%f\n", Vi);
      disparando = false;
    }
    //----------------------------------------------------------------------------------
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (!disparando && deltaPos.x != 0 && deltaPos.y != 0) {
      DrawLineV(posInicial, posFinal, RED);
      DrawCircle((int)Xpos, (int)Ypos, 20, RED);
    }

    EndDrawing();
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
