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
#define GRAV 9
#define CAMBIO_ALTURA 50
#define MAX_VI 100
#define CYAN                                                                   \
  CLITERAL(Color) { 0, 255, 255, 255 }
//------------------------------------------------------------------------------------
// Declaracion de funciones
//--------------------------------------------------------------------------------------
static void DEBUG_DIBUJOS(void);
static void RegenerarTerreno(void);
static void RegenerarJugadores(void);

static void Disparar(void);
static void ActualizarPelota(void);

static void DibujarCuadricula(void);
static void DibujarTerreno(void);
static void DibujarArrastre(void);
static void DibujarPelota(void);
static void DibujarJugadores(void);

static void Explosion(int a);

static void ReiniciarPelota(void);

//------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------
typedef struct Terreno {
  Rectangle rectangulo;
  Color color;
} Terreno;

typedef struct Jugador {
  int nRec;
  Rectangle rectangulo;
  Color color;
} Jugador;

typedef struct Pelota {
  Vector2 pos;
} Pelota;

//------------------------------------------------------------------------------------
// Inicializacion de variables
//--------------------------------------------------------------------------------------
static const int anchoPantalla = 1280;
static const int alturaPantalla = 720;

static Pelota pelota = {0};
static Jugador jugadores[2] = {0};
#if REALISTA
static float Xpos = 0.0;
static float Ypos = 0.0;
#endif
static int turno;

static float Vy = 0.0; // Velocidad de la pelota en Y
static float Vx = 0.0; // Velocidad de la pelota en X

static float Vi = 0.0; // Velocidad inicial

static float theta = 0; // Angulo de tiro

static Vector2 p1 = {0, 0};       // Punto Inicial (click)
static Vector2 p2 = {0, 0};       // Punto final (release del click)
static Vector2 deltaPos = {0, 0}; // El vector creado por estos dos puntos

static bool pausa = false;      // Pausado?
static bool disparando = false; // Se presiono click?
static bool victoria = false;
static Terreno edificio[32] = {0};

int main(void) {
  jugadores[0].color = CYAN;
  jugadores[1].color = RED;

  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
  InitWindow(anchoPantalla, alturaPantalla, "PAJAROS ENOJONES");
  InitAudioDevice();
  // EFECTOS
  Sound boom = LoadSound("resources/boom.wav");
  RegenerarTerreno();
  RegenerarJugadores();

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
      RegenerarJugadores();
      victoria = false;
    }

    if (disparando && !pausa) {
      ActualizarPelota();
      for (int i = 0; i < 32; i++) {
        bool colision =
            CheckCollisionCircleRec(pelota.pos, 20, edificio[i].rectangulo);
        if (colision) {
          Explosion(i);
          PlaySound(boom);
          ReiniciarPelota();
          turno = (turno == 0) ? 1 : 0;
        }
      }

      for (int i = 0; i < 2; i++) {
        bool colision =
            CheckCollisionCircleRec(pelota.pos, 20, jugadores[i].rectangulo);
        if (colision && turno != i) {
          ReiniciarPelota();
          victoria = true;
        }
      }
    }

    if (pelota.pos.x > anchoPantalla || pelota.pos.x < 0) {
      ReiniciarPelota();
      turno = (turno == 0) ? 1 : 0;
    }
    if (pelota.pos.y > alturaPantalla) {
      ReiniciarPelota();
      turno = (turno == 0) ? 1 : 0;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      if (turno == 0) {
        p1 = (Vector2){jugadores[0].rectangulo.x + 20,
                       jugadores[0].rectangulo.y - 20};
      } else {

        p1 = (Vector2){jugadores[1].rectangulo.x + 20,
                       jugadores[1].rectangulo.y - 20};
      }
    }
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
      Disparar();
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

    if (victoria) {
      DrawText(TextFormat("GANA EL JUGADOR %d!", turno + 1),
               (anchoPantalla / 2 - 250), (alturaPantalla / 2 - 60), 50, GOLD);
    }

    DrawText(TextFormat("Jugador %d dispara", turno + 1), 10, 30, 20, BLACK);

    if (disparando && deltaPos.x != 0 && deltaPos.y != 0) {
      DibujarPelota();
    }
    DibujarJugadores();
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      DibujarArrastre();
    }

    DrawFPS(10, 10);
    EndDrawing();
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  UnloadSound(boom);
  CloseAudioDevice();
  CloseWindow(); // Close window and OpenGL context
  //--------------------------------------------------------------------------------------

  return 0;
}

static void DEBUG_DIBUJOS(void) {
  DrawText(
      TextFormat("[%.0f,%.0f]", GetMousePosition().x, GetMousePosition().y),
      GetMousePosition().x + 15, GetMousePosition().y - 15, 30, CYAN);
}

static void Disparar(void) {
  for (int i = 0; i < 32; i++) {
    bool colision = CheckCollisionCircleRec(p1, 20, edificio[i].rectangulo);
    if (colision) {
      disparando = false;
      return;
    }
  }

  p2 = GetMousePosition();
  deltaPos = Vector2Subtract(p1, p2);

  pelota.pos = p1;

  theta = -atan2f(deltaPos.y, deltaPos.x);
  Vi = sqrtf(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y) / 2;
  Vi = (Vi > MAX_VI) ? MAX_VI : Vi;

  ///////////////////////////////////////////////////////////////////////
  // metodo de EULER !!!!!!!!!!!!!!!!!!!!!!!
  // (REF: https://fwww.physics.umd.edu/hep/drew/numerical_integration.html )
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

  disparando = true;
#if REALISTA
  Xpos = 0;
  Ypos = p1.y;
#endif /* ifdef REALISTA */
}

static void RegenerarJugadores(void) {
  jugadores[0].nRec = GetRandomValue(2, 8);
  jugadores[1].nRec = GetRandomValue(24, 30);
  for (int i = 0; i < 2; i++) {
    jugadores[i].rectangulo.x = edificio[jugadores[i].nRec].rectangulo.x;
    jugadores[i].rectangulo.width = 40;
    jugadores[i].rectangulo.y = edificio[jugadores[i].nRec].rectangulo.y - 40;
    jugadores[i].rectangulo.height = 40;
  }
}

static void ActualizarPelota(void) {
  pelota.pos.x += Vx * GetFrameTime() * (float)FF_T; // x = x_0 + dt*Vx_0
  pelota.pos.y += Vy * GetFrameTime() * (float)FF_T; // y = y_0 + dt*Vy
  Vy += GRAV * GetFrameTime() * (float)FF_T;         // + y''*dt para Vy
#if REALISTA
  Xpos += Vi * cosf(theta) * GetFrameTime() * (float)FF_T;
  Ypos = p1.y - ((tanf(theta) * Xpos) -
                 ((GRAV / 2) * (1 / (Vi * Vi)) *
                  (1 / (cosf(theta) * cosf(theta))) * (Xpos * Xpos)));
#endif /* ifdef REALISTA */
}

static void DibujarCuadricula(void) {
  for (int i = 0; i < anchoPantalla / TAMAÑO_RECTANGULOS + 1; i++) {
    DrawLineV((Vector2){(float)TAMAÑO_RECTANGULOS * i, 0},
              (Vector2){(float)TAMAÑO_RECTANGULOS * i, (float)alturaPantalla},
              LIGHTGRAY);
  }

  for (int i = 0; i < alturaPantalla / TAMAÑO_RECTANGULOS + 1; i++) {
    DrawLineV((Vector2){0, (float)TAMAÑO_RECTANGULOS * i},
              (Vector2){(float)anchoPantalla, (float)TAMAÑO_RECTANGULOS * i},
              LIGHTGRAY);
  }
}

static void DibujarTerreno(void) {
  for (int i = 0; i < 32; i++) {
    DrawRectangleRec(edificio[i].rectangulo, edificio[i].color);
  }
}
static void DibujarJugadores(void) {
  DrawRectangleRec(jugadores[0].rectangulo, jugadores[0].color);
  DrawRectangleRec(jugadores[1].rectangulo, jugadores[1].color);
}

static void RegenerarTerreno(void) {
  int color1 = 0;
  int color2 = GetRandomValue(1, 4);

  edificio[0].rectangulo.height = GetRandomValue(300, 500);
  edificio[0].rectangulo.y = GetScreenHeight() - edificio[0].rectangulo.height;

  for (int i = 0; i < 32; i++) {
    color1 = color2;
    edificio[i].rectangulo.x = i * 40;
    edificio[i].rectangulo.width = 40;
    edificio[i].color =
        (Color){color1 * 20 + 60, color1 * 20 + 60, color1 * 20 + 60, 255};
    do {
      color2 = GetRandomValue(1, 4);
    } while (color1 == color2);
  }

  for (int i = 1; i < 32; i++) {
    int alturaAnterior = edificio[i - 1].rectangulo.height;

    edificio[i].rectangulo.x = i * 40;
    edificio[i].rectangulo.width = 40;

    if ((GetRandomValue(0, 1) == 1 && alturaAnterior > 100) ||
        alturaAnterior > 500 ||
        (edificio[i - 2].rectangulo.height >
             edificio[i - 1].rectangulo.height &&
         edificio[i - 3].rectangulo.height <
             edificio[i - 2].rectangulo.height)) {
      edificio[i].rectangulo.height = alturaAnterior - CAMBIO_ALTURA;
    } else {
      edificio[i].rectangulo.height = alturaAnterior + CAMBIO_ALTURA;
    }
    edificio[i].rectangulo.y =
        GetScreenHeight() - edificio[i].rectangulo.height;
  }
}

static void DibujarArrastre(void) {
  p2 = GetMousePosition();
  deltaPos = Vector2Subtract(p1, p2);
  float f_mouse = sqrt(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y) / 2;
  float ang_mouse = -atan2f(deltaPos.y, deltaPos.x) * RAD2DEG;
  if (f_mouse > MAX_VI) {
    f_mouse = MAX_VI;
  }

  DrawText(TextFormat("[%.2f,%.2f°]", f_mouse, ang_mouse), p1.x - 50, p1.y - 20,
           20, RED);
  DrawLineEx(p1, GetMousePosition(), 2, RED);
}

static void DibujarPelota(void) {
  DrawCircle((int)pelota.pos.x, (int)pelota.pos.y, 20, (Color){255, 0, 0, 120});
#if REALISTA
  DrawCircle((int)Xpos + p1.x, (int)Ypos, 20, (Color){0, 255, 255, 120});
#endif /* ifdef REALISTA */
}

static void Explosion(int a) {
  if (a != jugadores[0].nRec && a != jugadores[1].nRec) {
    edificio[a].rectangulo.y += 50;
    edificio[a].rectangulo.height -= 50;
  }
  if (a >= 1 && (a - 1) != jugadores[0].nRec && (a - 1) != jugadores[1].nRec) {
    edificio[a - 1].rectangulo.y += 25;
    edificio[a - 1].rectangulo.height -= 25;
  }

  if (a <= 31 && (a + 1) != jugadores[0].nRec && (a + 1) != jugadores[1].nRec) {
    edificio[a + 1].rectangulo.y += 25;
    edificio[a + 1].rectangulo.height -= 25;
  }
}

static void ReiniciarPelota(void) {
  pelota.pos.x = 0;
  pelota.pos.y = 0;

  deltaPos.x = 0;
  deltaPos.y = 0;

  disparando = false;
}
