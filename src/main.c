#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
//------------------------------------------------------------------------------------
// CONSTANTES
//--------------------------------------------------------------------------------------
#define REALISTA 0
#define TAMAÑO_RECTANGULOS 80
#define FF_T 20
#define GRAV 8
#define CAMBIO_ALTURA 50
#define MAX_VI 120
#define DIV_POTENCIA 2
#define MAX_DISTANCIA 400
#define CYAN                                                                                                 \
    CLITERAL(Color) { 0, 255, 255, 255 }
#define JULIAN                                                                                               \
    CLITERAL(Color) { 255, 0, 255, 255 }
//------------------------------------------------------------------------------------
// Declaracion de funciones
//--------------------------------------------------------------------------------------
static void RegenerarTerreno(void);
static void RegenerarJugadores(void);
static void ReiniciarPelota(void);
static void SiguienteTurno(void);
static void Disparar(void);
static void DibujarJuego(void);
static void ActualizarPelota(void);
static void DibujarCuadricula(void);
static void DibujarTerreno(void);
static void DibujarArrastre(void);
static void DibujarPelota(void);

static void EscribirArchivo(void);
static void LeerLeaderboard(void);
static void GuardarPuntaje(void);
static void InicializarArchivo(void);

static void DibujarJugadores(void);
static void Explosion(int a);

static void TEST(void);

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
    char nombre[11];
} Jugador;

typedef struct Pelota {
    Vector2 pos;
} Pelota;

typedef struct Leaderboard {
    char nombre[11];
    int puntuacion;
} Leaderboard;

//------------------------------------------------------------------------------------
// Inicializacion de variables
//--------------------------------------------------------------------------------------
static const int anchoPantalla = 1280;
static const int alturaPantalla = 720;

static Pelota pelota = {0};        // PElota pelota pelota
static Jugador jugadores[2] = {0}; // Jugadores (rectangulos y propiedades)
static Leaderboard lb[10] = {0};   // Leaderboard en memoria

#if REALISTA
static float Xpos = 0.0;
static float Ypos = 0.0;
#endif
static int turno = 0; // A que jugador le toca tirar

static float Vy = 0.0; // Velocidad de la pelota en Y
static float Vx = 0.0; // Velocidad de la pelota en X

static float Vi = 0.0; // Velocidad inicial

static float theta = 0; // Angulo de tiro

static Vector2 punto1Arrastre = {0, 0}; // Punto Inicial (click)
static Vector2 punto2Arrastre = {0, 0}; // Punto final (release del click)
static Vector2 vectorArrastre = {0, 0}; // El vector creado por estos dos puntos

static bool pausa = false;         // Pausado?
static bool disparando = false;    // Se presiono click?
static bool victoria = false;      // A terminado la partida?
static Terreno edificio[32] = {0}; // Arreglo de edificios

static double tiempoInicial = 0; // Tiempo de inicio de la partida
static Rectangle cajaTexto = {(anchoPantalla - 250) / 2.0f, (alturaPantalla / 2.0f - 50), 250, 50};
static bool iniciando = true;

// Puntaje calculado segun el tiempo que se tarda en terminar la partida
static int puntaje = 0;
int main(void) {

    // INICIALIZAR JUGADORES
    jugadores[0].color = CYAN;

    strncpy(jugadores[0].nombre, "Cyan", 10);
    strncpy(jugadores[1].nombre, "Julian", 10);

    jugadores[1].color = RED;
    int contadorLetras = strlen(jugadores[0].nombre);

    // INICIALIZAR CONFIGURACION
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);

    // INICIALIZAR COMPONENTES
    InitWindow(anchoPantalla, alturaPantalla, "PAJAROS ENOJONES");

    int animFrames = 0;

    Image imLogoAnim = LoadImageAnim("resources/logo.gif", &animFrames);

    Texture2D texLogoAnim = LoadTextureFromImage(imLogoAnim);

    unsigned int nextFrameDataOffset = 0;

    int currentAnimFrame = 0;
    int frameDelay = 8;
    int frameCounter = 0;

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    InitAudioDevice();

    // CARGAR EFECTOS
    Sound boom = LoadSound("resources/boom.wav");
    // CARGAR LEADERBOARD
    LeerLeaderboard();

    // INICIALIZAR ESTADO DEL JUEGO
    RegenerarTerreno();
    RegenerarJugadores();

    //--------------------------------------------------------------------------------------
    // Juego
    //--------------------------------------------------------------------------------------
    while (!WindowShouldClose()) {
        //----------------------------------------------------------------------------------
        // Update
        //----------------------------------------------------------------------------------

        // Solo se ejecuta cuando los Jugadores ya tienen nombres.
        if (!iniciando) {
            // LOGICA DEL JUEGO

            // Si se presioan la tecla P, pausar el juego.
            if (IsKeyPressed(KEY_P)) {
                printf("Pausa\n");
                pausa = !pausa;
            }

            //------ DEBUGGING
            if (IsKeyPressed(KEY_T)) {
                printf("TEST\n");
                TEST();
            }
            if (IsKeyPressed(KEY_G)) {
                printf("GUARDADO\n");
                GuardarPuntaje();
            }
            if (IsKeyPressed(KEY_E)) {
                printf("ESCRITO!\n");
                EscribirArchivo();
            }
            if (IsKeyPressed(KEY_W)) {
                printf("win\n");
                ReiniciarPelota();
                victoria = true;
            }
            //----- DEBUGGING

            // Reiniciar partida
            if (IsKeyPressed(KEY_SPACE)) {
                printf("Regen\n");
                RegenerarTerreno();
                RegenerarJugadores();
                victoria = false;
                tiempoInicial = GetTime();
            }

            // Logica de actualizar pelota y checar colisiones
            // Solo ocurre cuando se realiza un disparo y no esta pausado el juego
            if (disparando && !pausa) {
                ActualizarPelota();
                // Comprobamos que no haya colisionado con ninguno de los rectangulos
                for (int i = 0; i < 32; i++) {
                    bool colision = CheckCollisionCircleRec(pelota.pos, 20, edificio[i].rectangulo);
                    // En el caso de que si exista una colision
                    if (colision) {
                        Explosion(i);
                        PlaySound(boom);
                        ReiniciarPelota();
                        SiguienteTurno();
                    }
                }

                // Comprobamos que no haya colisiones con jugadores
                for (int i = 0; i < 2; i++) {
                    bool colision = CheckCollisionCircleRec(pelota.pos, 20, jugadores[i].rectangulo);
                    // En el caso de golpear el jugador enemigo
                    if (colision && turno != i) {
                        ReiniciarPelota();
                        victoria = true;
                        puntaje = 500 - (int)(GetTime() - tiempoInicial);
                        GuardarPuntaje();
                    }
                }
            }

            // Checar si la pelota no rebaso el borde de la ventana en x
            if (pelota.pos.x > anchoPantalla || pelota.pos.x < 0) {
                ReiniciarPelota();
                SiguienteTurno();
            }

            // Checar si la pelota no rebaso el piso
            if (pelota.pos.y > alturaPantalla) {
                ReiniciarPelota();
                SiguienteTurno();
            }

            // Al dejar de arrastrar
            if (!disparando && !victoria) {
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
                    // Si el puntero no esta lejos del jugador
                    if (Vector2Distance(punto1Arrastre, GetMousePosition()) < MAX_DISTANCIA) {
                        Disparar();
                    }
                }
            }
        } else {
            // En caso contrario obtenemos sus nombres...
            // Logica para textInput
            // REF: https://www.raylib.com/examples/text/loader.html

            // Obtiene el codigo de caracter
            int key = GetCharPressed();
            while (key > 0) {
                // Solo permitimos caracteres de A-z y especiales
                if ((key >= 65) && (key <= 122) && (contadorLetras < 10)) {
                    jugadores[turno].nombre[contadorLetras] = (char)key;
                    jugadores[turno].nombre[contadorLetras + 1] = '\0';
                    contadorLetras++;
                }
                key = GetCharPressed();
            }
            // Borrar caracter
            if (IsKeyPressed(KEY_BACKSPACE)) {
                contadorLetras--;
                if (contadorLetras < 0)
                    contadorLetras = 0;
                jugadores[turno].nombre[contadorLetras] = '\0';
            }

            // Guardar el nombre y continuar con el siguiente
            if (IsKeyPressed(KEY_ENTER)) {
                if (turno != 1 && contadorLetras > 0) {
                    contadorLetras = strlen(jugadores[1].nombre);
                    turno = 1;
                } else if (contadorLetras > 0 && turno == 1) {
                    iniciando = false;
                    turno = GetRandomValue(0, 1);
                    punto1Arrastre =
                        (Vector2){jugadores[turno].rectangulo.x + 20, jugadores[turno].rectangulo.y - 20};
                }
            }

            frameCounter++;
            if (frameCounter >= frameDelay) {
                currentAnimFrame++;
                if (currentAnimFrame >= animFrames)
                    currentAnimFrame = 0;

                nextFrameDataOffset = imLogoAnim.width * imLogoAnim.height * 4 * currentAnimFrame;

                UpdateTexture(texLogoAnim, ((unsigned char *)imLogoAnim.data) + nextFrameDataOffset);

                frameCounter = 0;
            }
        }

        //----------------------------------------------------------------------------------
        // Dibujar
        //----------------------------------------------------------------------------------
        BeginDrawing();
        //-------------------------------------------------------------------------------------

        ClearBackground(RAYWHITE);

        // Si ya tenemos los nombres, dibujamos el juego real!
        if (!iniciando) {
            DibujarJuego();
        } else {
            DrawTexture(texLogoAnim, (anchoPantalla - texLogoAnim.width) / 2, 100, WHITE);

            DrawText("el videojuego", (anchoPantalla - MeasureText("el videojuego", 20)) / 2, 180, 20, RED);
            const char *txtNombre = TextFormat("Escribe el nombre del jugador %d!", turno + 1);
            int offsetNombre = MeasureText(txtNombre, 25);

            // En caso contrario escribimos el input del usuario.
            DrawText(txtNombre, (anchoPantalla - offsetNombre) / 2, (int)cajaTexto.y - 40, 25, BLACK);

            DrawRectangleRec(cajaTexto, LIGHTGRAY);
            DrawText(jugadores[turno].nombre, (int)cajaTexto.x + 5, (int)cajaTexto.y + 8, 40,
                     jugadores[turno].color);

            DrawText("Una produccion original de:",
                     (anchoPantalla - MeasureText("Una produccion original de", 20)) / 2, 550, 20, GRAY);

            DrawText("Jose Julian Parra Virgen",
                     (anchoPantalla - MeasureText("Jose Julian Parra Virgen", 15)) / 2, 580, 15, GRAY);
            DrawText("Cyan Mach", (anchoPantalla - MeasureText("Cyan Mach", 15)) / 2, 600, 15, GRAY);
            DrawText("Luis Alberto Garcia Lopez",
                     (anchoPantalla - MeasureText("Luis Alberto Garcia Lopez", 15)) / 2, 620, 15, GRAY);
            DrawText("Carlos Adrian Navarro Navarro",
                     (anchoPantalla - MeasureText("Carlos Adrian Navarro Navarro", 15)) / 2, 640, 15, GRAY);
        }

        //--------------------------------------------------------------------------------------
        EndDrawing();
        //--------------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    EscribirArchivo(); // Guardar Leaderboard
    UnloadSound(boom);
    UnloadImage(imLogoAnim);
    UnloadTexture(texLogoAnim);
    CloseAudioDevice();
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

static void DibujarJuego(void) {
    DibujarCuadricula();
    DibujarTerreno();
    DibujarJugadores();

    // Solo dibujamos si se a disparado y el arrastre tiene magnitud
    if (disparando && Vector2Length(vectorArrastre) != 0) {
        DibujarPelota();
    }

    // Solo dibujamos si tiene sentido y no esta muy lejos el mouse del jugador
    if (!disparando && !victoria && Vector2Distance(punto1Arrastre, GetMousePosition()) < MAX_DISTANCIA) {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            DibujarArrastre();
        }
    }

    DrawFPS(10, 10);

    if (pausa) {
        DrawText("PAUSADO", (anchoPantalla / 2 - 100), 20, 50, RED);
    }

    // En el caso que un jugador gane
    if (victoria) {
        // Dibujamos un fondo transparente
        DrawRectangle(0, 0, anchoPantalla, alturaPantalla, (Color){245, 245, 245, 100});

        const char *textoVictoria = TextFormat("GANA %s CON %d PUNTOS!", jugadores[turno].nombre, puntaje);
        int offsetTV = MeasureText(textoVictoria, 50);
        DrawText(textoVictoria, ((anchoPantalla - offsetTV) / 2), (alturaPantalla / 2 - 80), 50, GOLD);

        const char *subTexto = "Presiona [SPACE] para reiniciar el juego...";
        int offsetSubt = MeasureText(subTexto, 20);
        DrawText(subTexto, ((anchoPantalla - offsetSubt) / 2), (alturaPantalla / 2 - 30), 20, GOLD);

        // Offset para centrar
        int offsetTexto = MeasureText("LEADERBOARD:", 20);

        // Dibujamos el leaderbaord
        DrawText("LEADERBOARD:", ((anchoPantalla - offsetTexto) / 2), (alturaPantalla / 2), 20, RED);
        // Solo 5 primeras entradas del leaderboard en memoria
        for (int i = 1; i <= 5; i++) {
            // Formateamos para que se vea bonito
            DrawText(TextFormat("%s", lb[i - 1].nombre), (anchoPantalla / 2 - 90),
                     (alturaPantalla / 2 + (25 * i)), 20, RED);
            DrawText(TextFormat("%d", lb[i - 1].puntuacion), (anchoPantalla / 2 + 60),
                     (alturaPantalla / 2 + (25 * i)), 20, RED);
        }
    }
    DrawText(TextFormat("%s dispara", jugadores[turno].nombre), 10, 30, 20, BLACK);

    if (!disparando && !victoria && Vector2Distance(punto1Arrastre, GetMousePosition()) >= MAX_DISTANCIA) {
        DrawText("ES MI TURNO!", punto1Arrastre.x - 70, punto1Arrastre.y - 15, 20, GRAY);
    }

    if (!victoria) {
        int segundos = (int)(GetTime() - tiempoInicial);
        const char *Tiempo = TextFormat("%d", segundos);
        int textWidth = MeasureText(Tiempo, 20);
        DrawText(Tiempo, (anchoPantalla / 2) - (textWidth / 2), 10, 20, BLACK);
    }
}

static void SiguienteTurno(void) {
    // Actualizamos el turno
    turno = (turno == 0) ? 1 : 0;
    // Y la posicion inicial de disparo
    punto1Arrastre = (Vector2){jugadores[turno].rectangulo.x + 20, jugadores[turno].rectangulo.y - 20};
}

static void Disparar(void) {

    // Punto actual del mouse
    punto2Arrastre = GetMousePosition();

    // El vector formado por los puntos
    vectorArrastre = Vector2Subtract(punto1Arrastre, punto2Arrastre);

    // Posicion inicial de la pelota
    pelota.pos = punto1Arrastre;

    // Calculamos el angulo que crea el vector de arrastre con el suelo
    theta = -atan2f(vectorArrastre.y, vectorArrastre.x);

    // Calculamos su magnitud y arreglamos para que el usuario pueda apuntar bien
    Vi = sqrtf(vectorArrastre.x * vectorArrastre.x + vectorArrastre.y * vectorArrastre.y) / DIV_POTENCIA;

    // Elejimos o la velocidad o la velocidad maxima
    Vi = (Vi > MAX_VI) ? MAX_VI : Vi;

    /* -----------------------------------------------------------------------------
     metodo de EULER !!!!!!!!!!!!!!!!!!!!!!!
     (REF: https://fwww.physics.umd.edu/hep/drew/numerical_integration.html )

     el movimiento de proyectiles es descrito por increibles EDOs...

     y'(0) = V_y0 = v_0sin(theta)
     x'(0) = V_x0 = v_0cos(theta)

     x' = Vx = V_x0; CONST
     y' = Vy = V_x0 - g*dt (y'' = a_y = -g)
    ------------------------------------------------------------------------------ */

    // Valores iniciales
    Vx = Vi * cosf(theta);
    Vy = -sinf(theta) * Vi; // Invertimos el signo pues el eje y esta invertido

    // Anunciar que estamos disparando
    disparando = true;

#if REALISTA
    Xpos = 0;
    Ypos = punto1Arrastre.y;
#endif
}

static void RegenerarJugadores(void) {
    // Elegimos un edificio en ciertas regiones del mapa
    jugadores[0].nRec = GetRandomValue(2, 8);
    jugadores[1].nRec = GetRandomValue(24, 30);

    // Ponemos a cada jugador encima de su respectivo edificio
    for (int i = 0; i < 2; i++) {
        jugadores[i].rectangulo.x = edificio[jugadores[i].nRec].rectangulo.x;
        jugadores[i].rectangulo.width = 40;
        jugadores[i].rectangulo.y = edificio[jugadores[i].nRec].rectangulo.y - 40;
        jugadores[i].rectangulo.height = 40;
    }

    // Reiniciamos el punto de arrastre al turno del jugador
    punto1Arrastre = (Vector2){jugadores[turno].rectangulo.x + 20, jugadores[turno].rectangulo.y - 20};
}

static void ActualizarPelota(void) {
    // Por metodo de Euler
    pelota.pos.x += Vx * GetFrameTime() * (float)FF_T; // x = x_0 + dt*Vx_0
    pelota.pos.y += Vy * GetFrameTime() * (float)FF_T; // y = y_0 + dt*Vy
    Vy += GRAV * GetFrameTime() * (float)FF_T;         // + y''*dt para Vy

#if REALISTA
    // Tiro parabolico
    Xpos += Vi * cosf(theta) * GetFrameTime() * (float)FF_T;
    Ypos = punto1Arrastre.y - ((tanf(theta) * Xpos) - ((GRAV / 2) * (1 / (Vi * Vi)) *
                                                       (1 / (cosf(theta) * cosf(theta))) * (Xpos * Xpos)));
#endif
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

static void DibujarJugadores(void) {
    DrawRectangleRec(jugadores[0].rectangulo, jugadores[0].color);
    DrawRectangleRec(jugadores[1].rectangulo, jugadores[1].color);
}

static void RegenerarTerreno(void) {
    // Valores que generaran colores aleatorios
    int color1 = 0;
    int color2 = GetRandomValue(1, 4);

    edificio[0].rectangulo.height = GetRandomValue(300, 500);
    edificio[0].rectangulo.y = GetScreenHeight() - edificio[0].rectangulo.height;

    for (int i = 0; i < 32; i++) {
        color1 = color2;

        // Los edifcios son equidistantes
        edificio[i].rectangulo.x = i * 40;
        edificio[i].rectangulo.width = 40;

        // Tenemos un color (verdoso) inicial, pero la intensidad cambia segun los valores aleatorios
        edificio[i].color = (Color){(color1 * 20 + 60), (color1 * 20 + 80), (color1 * 20 + 60), 255};

        // Evitamos colores seguidos
        do {
            color2 = GetRandomValue(1, 4);
        } while (color1 == color2);
    }

    for (int i = 1; i < 32; i++) {
        // Utilizamos alturas anteriores para comparar
        int alturaAnterior = edificio[i - 1].rectangulo.height;

        // Condicion de resta de altura
        bool condicion = (GetRandomValue(0, 1) == 1 && alturaAnterior > 100) || alturaAnterior > 500 ||
                         (i > 2 && edificio[i - 2].rectangulo.height > edificio[i - 1].rectangulo.height &&
                          edificio[i - 3].rectangulo.height < edificio[i - 2].rectangulo.height);

        if (condicion) {
            edificio[i].rectangulo.height = alturaAnterior - CAMBIO_ALTURA;
        } else {
            edificio[i].rectangulo.height = alturaAnterior + CAMBIO_ALTURA;
        }

        // Necesario para dibujar los edificios correctamente
        edificio[i].rectangulo.y = GetScreenHeight() - edificio[i].rectangulo.height;
    }
}

static void DibujarArrastre(void) {

    // Obtenemos el punto (final) hasta el que se esta arrastrando
    punto2Arrastre = GetMousePosition();

    // Trazamos un vector con el punto de inicio y el final
    vectorArrastre = Vector2Subtract(punto2Arrastre, punto1Arrastre);

    // Obtenemos la normal del vector (direccion del arrastre)
    Vector2 direccionArrastre = Vector2Normalize(vectorArrastre);

    // La fuerza depende de que tanto a sido jalado
    // Ajustamos para que sea mas comodo disparar con DIV_POTENCIA
    float fuerza = Vector2Length(vectorArrastre) / DIV_POTENCIA;

    float angulo = atan2f(vectorArrastre.y, vectorArrastre.x) * RAD2DEG;

    // Obtenemos el valor minimo
    if (fuerza > MAX_VI) {
        fuerza = MAX_VI;
    }

    // Ec. linea dado punto y direccion.
    // Estiramos hasta que la magnitud sea igual a la fuerza max o a la de arrastre
    // Corregimos para el ajuste inicial
    Vector2 posFinal = Vector2Add(punto1Arrastre, (Vector2Scale(direccionArrastre, fuerza * DIV_POTENCIA)));

    // Dibujamos los valores
    DrawText(TextFormat("[%.2f,%.2f°]", fuerza, angulo), punto1Arrastre.x - 50, punto1Arrastre.y - 20, 20,
             RED);
    // Y la linea
    DrawLineEx(punto1Arrastre, posFinal, 2, RED);
}

static void DibujarPelota(void) {
    DrawCircle((int)pelota.pos.x, (int)pelota.pos.y, 20, (Color){255, 0, 0, 120});
#if REALISTA
    DrawCircle((int)Xpos + punto1Arrastre.x, (int)Ypos, 20, (Color){0, 255, 255, 120});
#endif /* ifdef REALISTA */
}

static void Explosion(int a) {
    // Disminuimos la altura del edifcio golpeado (si es posible) y su vecindad (si es posible)
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

    vectorArrastre.x = 0;
    vectorArrastre.y = 0;

    disparando = false;
}

static void LeerLeaderboard(void) {
    FILE *fptr;
    fptr = fopen("lb.txt", "r");

    // Valores de lectura temporal
    char nombre[11];
    int puntuacion;

    // Sera verdadero en el caso que el archivo no exista
    if (fptr != NULL) {
        printf("===== ARCHIVO LEIDO =====\n");

        // Lemos 10 lineas del archivo
        for (int i = 0; i < 10; i++) {
            // Escanemos el texto
            // Esperamos la estructura: str:NOMBRE int:PUNTACION
            if (fscanf(fptr, "%s %d", nombre, &puntuacion) != EOF) {
                printf("N: %s P: %d\n", nombre, puntuacion);
                // Guardamos los valores leidos
                strncpy(lb[i].nombre, nombre, 10);
                lb[i].puntuacion = puntuacion;
            }
        }
    } else {
        // Creamos el archivo si no existe
        printf("===== ARCHIVO CREADO =====\n");
        InicializarArchivo();
        return;
    }
    fclose(fptr);
}

static void InicializarArchivo(void) {
    FILE *fptr;
    // Al leer, se crea el archivo (flag w)
    fptr = fopen("lb.txt", "w");

    if (fptr != NULL) {
        for (int i = 0; i < 10; i++) {
            // Escribimos nombre y puntuacion como 0 en el archivo
            fprintf(fptr, "0 0\n");
            // Inicializamos el leaderboard en memoria
            strncpy(lb[i].nombre, "0", 10);
            lb[i].puntuacion = 0;
        }
    }
    fclose(fptr);
}

static void GuardarPuntaje(void) {

    for (int i = 0; i < 10; i++) {
        // Si el puntaje es mas alto los guardados
        if (puntaje > lb[i].puntuacion) {
            // Shifteamos hacia abajo todo el leaderboard
            for (int j = 9; j > i; j--) {
                strncpy(lb[j].nombre, lb[j - 1].nombre, 10);
                lb[j].puntuacion = lb[j].puntuacion;
            }
            // Guardamos el nuevo puntaje
            strncpy(lb[i].nombre, jugadores[turno].nombre, 10);
            lb[i].puntuacion = puntaje;
            break;
        }
    }
}

static void EscribirArchivo(void) {
    FILE *fptr;
    // Modo escritura
    fptr = fopen("lb.txt", "w");

    // Si no hay errores...
    if (fptr != NULL) {
        // Escribimos 10 lineas
        for (int i = 0; i < 10; i++) {
            // Asumimos que leaderboard ya esta bien definido
            fprintf(fptr, "%s %d\n", lb[i].nombre, lb[i].puntuacion);
        }
        printf("====ARCHIVO GUARDADO====\n");
    }
    fclose(fptr);
}

// Obtenemos valores en memoria
static void TEST(void) {
    for (int i = 0; i < 10; i++) {
        printf("N: %s P: %d\n", lb[i].nombre, lb[i].puntuacion);
    }
}
