#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>

#define REALISTA 0
#define TAMAÑO_RECTANGULOS 80
#define FF_T 20

void InitV0(float *theta, float *Vi, float *Vy, float *Vx) {
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
  *Vx = cosf(*theta) * *Vi;
  *Vy = -sinf(*theta) * *Vi; // Invertimos el signo pues el eje y esta invertido
}

void Cero(float *x, float *y, float *dx, float *dy) {
  *x = 0;
  *y = 0;
  *dx = 0;
  *dy = 0;
}
//------------------------------------------------------------------------------------
// Pajaros Enojones
//------------------------------------------------------------------------------------
int main(void) {
  // Inicializacion
  //--------------------------------------------------------------------------------------
  const int anchoPantalla = 1920 + 1;
  const int alturaPantalla = 1080 + 1;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(anchoPantalla, alturaPantalla, "PAJAROS ENOJONES");

  Rectangle objetivo = {80 * 21, GetScreenHeight() - 80 * 3, 80 * 3, 80 * 3};
  float X_pelota = 0.0; // Posicion de la bola en X
  float Y_pelota = 0.0; // Posicion de la bola en Y

#if REALISTA
  float Xpos = 0.0;
  float Ypos = 0.0;
#endif /* ifdef REALISTA */

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

    if (IsKeyPressed(KEY_SPACE)) {

      printf("Pausa\n");
      pausa = !pausa;
    }

    if (!disparando && !pausa) {
      X_pelota += Vx * GetFrameTime() * (float)FF_T; // y = y_0 + dt*Vy
      Y_pelota += Vy * GetFrameTime() * (float)FF_T; // x = x_0 + dt*Vx_0

      Vy += 9.8 * GetFrameTime() * (float)FF_T; // + y''*dt para Vy

#if REALISTA
      Xpos += Vi * cosf(theta) * GetFrameTime() * (float)FF_T;
      Ypos = p1.y - ((tanf(theta) * Xpos) -
                     (4.9 * (1 / (Vi * Vi)) * (1 / (cosf(theta) * cosf(theta))) * (Xpos * Xpos)));
#endif /* ifdef REALISTA */
    }

    if (X_pelota > GetScreenWidth() || X_pelota < 0) {
      Cero(&X_pelota, &Y_pelota, &deltaPos.x, &deltaPos.y);
      disparando = false;
    }
    if (Y_pelota > GetScreenHeight()) {
      Cero(&X_pelota, &Y_pelota, &deltaPos.x, &deltaPos.y);
      disparando = false;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !disparando) {
      p1 = GetMousePosition();
      disparando = true;
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      p2 = GetMousePosition();

      deltaPos = Vector2Subtract(p1, p2);

      X_pelota = p1.x;
      Y_pelota = p1.y;

      theta = -atan2f(deltaPos.y, deltaPos.x);

      Vi = sqrtf(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y) / 2;
      Vi = (Vi > 150) ? 150 : Vi;
      InitV0(&theta, &Vi, &Vy, &Vx);

#if REALISTA
      Xpos = 0;
      Ypos = p1.y;
#endif /* ifdef REALISTA */

      disparando = false;
    }

    bool colision = CheckCollisionCircleRec((Vector2){X_pelota, Y_pelota}, 20, objetivo);

    if (colision) {
      pausa = true;
    }
    //----------------------------------------------------------------------------------
    // Dibujar
    //----------------------------------------------------------------------------------
    BeginDrawing();
    DrawFPS(10, 10);

    ClearBackground(RAYWHITE);

    DrawRectangleRec(objetivo, GREEN);

    for (int i = 0; i < anchoPantalla / TAMAÑO_RECTANGULOS + 1; i++) {
      DrawLineV((Vector2){(float)TAMAÑO_RECTANGULOS * i, 0},
                (Vector2){(float)TAMAÑO_RECTANGULOS * i, (float)alturaPantalla}, LIGHTGRAY);
    }

    for (int i = 0; i < alturaPantalla / TAMAÑO_RECTANGULOS + 1; i++) {
      DrawLineV((Vector2){0, (float)TAMAÑO_RECTANGULOS * i},
                (Vector2){(float)anchoPantalla, (float)TAMAÑO_RECTANGULOS * i}, LIGHTGRAY);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      p2 = GetMousePosition();
      deltaPos = Vector2Subtract(p1, p2);
      float f_mouse = sqrt(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y) / 2;
      float ang_mouse = -atan2f(deltaPos.y, deltaPos.x) * RAD2DEG;
      if (f_mouse > 150) {
        f_mouse = 150;
      }
      DrawText(TextFormat("[%.2f,%.2f°]", f_mouse, ang_mouse), GetMousePosition().x + 15,
               GetMousePosition().y + 15, 30, RED);
      DrawLineEx(p1, GetMousePosition(), 10, RED);
    }
    if (pausa) {
      DrawText("PAUSADO", (GetScreenWidth() / 2 - 100), 20, 50, RED);
    }
    if (!disparando && deltaPos.x != 0 && deltaPos.y != 0) {
      DrawCircle((int)X_pelota, (int)Y_pelota, 20, (Color){255, 0, 0, 120});
#if REALISTA
      DrawCircle((int)Xpos + p1.x, (int)Ypos, 20, (Color){0, 255, 255, 120});
#endif /* ifdef REALISTA */
    }

    EndDrawing();
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}
