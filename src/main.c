#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
//------------------------------------------------------------------------------------
// CONSTANTES
//--------------------------------------------------------------------------------------
#define REALISTA 0
#define TAMAÑO_RECTANGULOS 80
#define FF_T 20

//------------------------------------------------------------------------------------
// Declaracion de funciones
//--------------------------------------------------------------------------------------
static void DEBUG_DIBUJOS(void);
static void RegenerarTerreno(void);

static void Disparar(void);
static void ActualizarPelota(void);

static void DibujarCuadricula(void);
static void DibujarTerreno(void);
static void DibujarArrastre(void);
static void DibujarPelota(void);

static void IniciarV(void);
static void ReiniciarPelota(void);

//------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

typedef struct Terreno {
  Rectangle rectangulo;
  Color color;
} Terreno;

typedef struct pelota {
  float x;
  float y;
} Pelota;

//------------------------------------------------------------------------------------
// Inicializacion de variables
//--------------------------------------------------------------------------------------
static const int anchoPantalla = 1280;
static const int alturaPantalla = 720;

static Pelota pelota = {0};

#if REALISTA
static float Xpos = 0.0;
static float Ypos = 0.0;
#endif

static float Vy = 0.0; // Velocidad de la pelota en Y
static float Vx = 0.0; // Velocidad de la pelota en X

static float Vi = 0.0; // Velocidad inicial

static float theta = 0; // Angulo de tiro

static Vector2 p1 = {0, 0};       // Punto Inicial (click)
static Vector2 p2 = {0, 0};       // Punto final (release del click)
static Vector2 deltaPos = {0, 0}; // El vector creado por estos dos puntos

static bool pausa = false;      // Pausado?
static bool disparando = false; // Se presiono click?

static Terreno edificio[32] = {0};

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(anchoPantalla, alturaPantalla, "PAJAROS ENOJONES");

  RegenerarTerreno();
  //--------------------------------------------------------------------------------------
  // Juego
  //--------------------------------------------------------------------------------------
  while (!WindowShouldClose()) {
    //----------------------------------------------------------------------------------
    // Update
    //----------------------------------------------------------------------------------
    if (IsKeyPressed(KEY_SPACE)) {
      printf("Pausa\n");
      pausa = !pausa;
    }
    if (IsKeyPressed(KEY_R)) {
      printf("Regen\n");
      RegenerarTerreno();
    }

    if (!disparando && !pausa) {
      ActualizarPelota();
    }

    if (pelota.x > anchoPantalla || pelota.x < 0) {
      ReiniciarPelota();
      disparando = false;
    }
    if (pelota.y > alturaPantalla) {
      ReiniciarPelota();
      disparando = false;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !disparando) {
      p1 = GetMousePosition();
      disparando = true;
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      Disparar();
      disparando = false;
    }

    //----------------------------------------------------------------------------------
    // Dibujar
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DibujarCuadricula();
    DibujarTerreno();

    if (pausa) {
      DrawText("PAUSADO", (anchoPantalla / 2 - 100), 20, 50, RED);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      DibujarArrastre();
    }

    if (!disparando && deltaPos.x != 0 && deltaPos.y != 0) {
      DibujarPelota();
    }

    DEBUG_DIBUJOS();

    DrawFPS(10, 10);
    EndDrawing();
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

static void DEBUG_DIBUJOS(void) {
  DrawText(TextFormat("[%.0f,%.0f]", GetMousePosition().x, GetMousePosition().y), GetMousePosition().x + 15,
           GetMousePosition().y - 15, 30, (Color){0, 255, 255, 250});
}

static void Disparar(void) {
  p2 = GetMousePosition();

  deltaPos = Vector2Subtract(p1, p2);

  pelota.x = p1.x;
  pelota.y = p1.y;

  theta = -atan2f(deltaPos.y, deltaPos.x);

  Vi = sqrtf(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y) / 2;
  Vi = (Vi > 150) ? 150 : Vi;

  IniciarV();

#if REALISTA
  Xpos = 0;
  Ypos = p1.y;
#endif /* ifdef REALISTA */
}

static void ActualizarPelota(void) {
  pelota.x += Vx * GetFrameTime() * (float)FF_T; // x = x_0 + dt*Vx_0
  pelota.y += Vy * GetFrameTime() * (float)FF_T; // y = y_0 + dt*Vy

  Vy += 9.8 * GetFrameTime() * (float)FF_T; // + y''*dt para Vy

#if REALISTA
  Xpos += Vi * cosf(theta) * GetFrameTime() * (float)FF_T;
  Ypos = p1.y -
         ((tanf(theta) * Xpos) - (4.9 * (1 / (Vi * Vi)) * (1 / (cosf(theta) * cosf(theta))) * (Xpos * Xpos)));
#endif /* ifdef REALISTA */
}

static void DibujarCuadricula(void) {
  for (int i = 0; i < anchoPantalla / TAMAÑO_RECTANGULOS + 1; i++) {
    DrawLineV((Vector2){(float)TAMAÑO_RECTANGULOS * i, 0},
              (Vector2){(float)TAMAÑO_RECTANGULOS * i, (float)alturaPantalla}, LIGHTGRAY);
  }

  for (int i = 0; i < alturaPantalla / TAMAÑO_RECTANGULOS + 1; i++) {
    DrawLineV((Vector2){0, (float)TAMAÑO_RECTANGULOS * i},
              (Vector2){(float)anchoPantalla, (float)TAMAÑO_RECTANGULOS * i}, LIGHTGRAY);
  }
}

static void DibujarTerreno(void) {
  for (int i = 0; i < 32; i++) {
    DrawRectangleRec(edificio[i].rectangulo, edificio[i].color);
  }
}

static void RegenerarTerreno(void) {
  //////witdh terrain generation
  for (int i = 0; i < 32; i++) {
    edificio[i].rectangulo.x = i * 40;
    edificio[i].rectangulo.width = 40;
    edificio[i].color = (Color){1, 1, 1, 255};
  }

  // for first rectangle
  edificio[0].rectangulo.height = GetRandomValue(200, 700);
  edificio[0].rectangulo.y = alturaPantalla - edificio[0].rectangulo.height;

  /////height terrain generation
  // if odd substract height from the previous rectangle
  // if even add height from previous rectangle
  for (int i = 1; i < 32; i++) {
    int moneda = GetRandomValue(0, 1);
    if (moneda == 1) {
      edificio[i].rectangulo.height = edificio[i - 1].rectangulo.height - (float)40;
      edificio[i].rectangulo.y = alturaPantalla - edificio[i].rectangulo.height;
    } else {
      edificio[i].rectangulo.height = edificio[i - 1].rectangulo.height + (float)40;
      edificio[i].rectangulo.y = alturaPantalla - edificio[i].rectangulo.height;
    }
  }
}

static void DibujarArrastre(void) {
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

static void DibujarPelota(void) {
  DrawCircle((int)pelota.x, (int)pelota.y, 20, (Color){255, 0, 0, 120});
#if REALISTA
  DrawCircle((int)Xpos + p1.x, (int)Ypos, 20, (Color){0, 255, 255, 120});
#endif /* ifdef REALISTA */
}

static void IniciarV(void) {
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
  Vx = Vi * cosf(theta);
  Vy = -sinf(theta) * Vi; // Invertimos el signo pues el eje y esta invertido
}

static void ReiniciarPelota(void) {
  pelota.x = 0;
  pelota.y = 0;

  deltaPos.x = 0;
  deltaPos.y = 0;
}
