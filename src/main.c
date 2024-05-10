#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>

//------------------------------------------------------------------------------------
// Pajaros Enojones
//------------------------------------------------------------------------------------
int main(void) {
  // Inicializacion
  //--------------------------------------------------------------------------------------
  const int anchoPantalla = 1080;
  const int alturaPantalla = 720;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(anchoPantalla, alturaPantalla, "PAJAROS ENOJONES");

  float X_pelota = 0.0; // Posicion de la bola en X
  float Y_pelota = 0.0; // Posicion de la bola en Y

  float Vy = 0.0; // Velocidad de la pelota en Y
  float Vx = 0.0; // Velocidad de la pelota en X

  float Vi = 0.0; // Velocidad inicial

  float theta = 0; // Angulo de tiro

  Vector2 p1 = {0, 0};       // Punto Inicial (click)
  Vector2 p2 = {0, 0};       // Punto final (release del click)
  Vector2 deltaPos = {0, 0}; // El vector creado por estos dos puntos

  bool pausa = false;      // Pausado?
  bool disparando = false; // Se presiono click?
  //--------------------------------------------------------------------------------------
  // Juego
  while (!WindowShouldClose()) {
    // Update
    //----------------------------------------------------------------------------------

    if (IsKeyDown(KEY_SPACE))
      pausa = !pausa;

    if (!pausa) {
      X_pelota += Vx; // y = y_0 + dt*Vy
      Y_pelota += Vy; // x = x_0 + dt*Vx_0
    }

    if (X_pelota > GetScreenWidth() || X_pelota < 0) {
      // TODO: FUNCION DE ZERO-TODO
      X_pelota = 0;
      Y_pelota = 0;
      deltaPos.x = 0;
      deltaPos.y = 0;
      pausa = !pausa;
    }
    if (Y_pelota > GetScreenHeight()) {
      X_pelota = 0;
      Y_pelota = 0;
      deltaPos.x = 0;
      deltaPos.y = 0;

      pausa = !pausa;
    }
    Vy += 9.8 * 0.01; // + y''*dt para Vy

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !disparando) {
      p1 = GetMousePosition();
      disparando = true;
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      pausa = false;

      // TODO: FUNCION PARA CALCULAR TODO ESTO DE MANERA MAS BONITA
      p2 = GetMousePosition();
      deltaPos = Vector2Subtract(p1, p2);
      X_pelota = p1.x;
      Y_pelota = p1.y;

      theta = -atan2f(deltaPos.y, deltaPos.x);

      Vi = sqrt(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y) / 2;

      if (Vi > 150) {
        Vi = 150;
      }
      ///////////////////////////////////////////////////////////////////////
      // metodo de EULER oilar !!!!!!!!!!!!!!!!!!!!!!!
      // (REF: https://www.physics.umd.edu/hep/drew/numerical_integration.html )
      //
      // el movimiento de proyectiles es descrito por increibles EDOs...
      //
      // y'(0) = V_y0 = v_0sin(theta)
      // x'(0) = V_x0 = v_0cos(theta)
      //
      // x' = Vx = V_x0; CONST
      // y' = Vy = V_x0 - g*dt (y'' = a_y = -g)
      ///////////////////////////////////////////////////////////////////////
      Vx = Vi * cos(theta) * 15 * 0.01;
      Vy = -sin(theta) * Vi * 15 * 0.01; // Invertimos el signo pues el eje y esta invertido

      printf("%f\n", Vi);
      disparando = false;
    }
    //----------------------------------------------------------------------------------
    // Dibujar
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      DrawLineEx(p1, GetMousePosition(), 10, RED);
    }

    if (!disparando && deltaPos.x != 0 && deltaPos.y != 0) {
      DrawCircle((int)X_pelota, (int)Y_pelota, 20, RED);
    }

    EndDrawing();
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
