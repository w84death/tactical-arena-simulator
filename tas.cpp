// ----------------------------------------
// TACTICAL ARENA SIMULATOR
// Krzysztof Jankowski <kj@p1x.in>
//
// (c)2015 P1X
// http://p1x.in
//
// Repo:
// https://github.com/w84death/tactical-arena-simulator
//
// Linux:
// gcc -Os tas.cpp -o tas.app -lglut -lGL -lGLU -lm
//
// OSX:
// gcc -o tas tas.cpp -framework GLUT -framework OpenGL
//
// ----------------------------------------

// LIBS
// ----------------------------------------

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <stdlib.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#include <stdio.h>
#include <time.h>
#include <math.h>

// SYSTEM SETTINGS
// ----------------------------------------------------------------------------

bool fullscreen_mode  = true;
char win_title[]      = "TACTICAL ARENA SIMULATOR";
static float VERSION  = 0.3f;
int win_width         = 512;
int win_height        = 384;
int win_x             = 256;
int win_y             = 100;

// RENDERING SETTINGS
// ----------------------------------------------------------------------------

static int FPS        = 60;
int refresh_ms        = 1000/FPS;

// CAMERA SETTINGS
// ----------------------------------------------------------------------------

static float FOV      = 60.0f;
int cam_mode          = 0;
float cam_speed       = 0.8f;
float cam_move_speed  = 0.6f;
float cam_look_move_speed  = 0.6f;
float cam_pos[]       = {16.0f, 16.0f, 10.0f, 16.0f, 16.0f, 32.0f};
float cam_look_pos[]  = {16.0f, 16.0f, 4.0f, 16.0f, 16.0f, 0.0f};
float cam_aim[]       = {0.0f, 0.0f};
float cam_fog_start   = 6.0f;
float cam_fog_end     = 40.0f;
float cam_clear_color[4] = {0.3f, 0.05f, 0.6f, 1.0f};

// ARENA SETTINGS
// ----------------------------------------------------------------------------
static int CELLS_ARRAY_SIZE[]     = {32, 32};
static int ENTITIES_SIZE_MAX      = 64;
static int ENTITIES_SIZE_START    = 8;
static int MAX_CELLS              = CELLS_ARRAY_SIZE[0]*CELLS_ARRAY_SIZE[1];
int half[]                        = {CELLS_ARRAY_SIZE[0] * 0.5, CELLS_ARRAY_SIZE[1] * 0.5};

float arena_array[32][32];
float entities[64][4]             = {{16, 16, 0, 0.8}};
bool entities_rotated[64];

static int REALTIME               = true;
static int LOGIC_FPS              = 1000/24;

static float RANDOM_MIN_COLOUR    = 0.05f;
static float RANDOM_MAX_COLOUR    = 0.2f;

static float GROUND               = 0.4f;
static float WALL                 = 0.5f;
static float WALL_MAX             = 1.0f;
static float LOWER                = 0.05f;
static float LOWER_P              = 0.1f;
static float EXPAND               = 0.001f;

static bool WALL_CAN_BE_DESTROYED = false;

int STATE                         = 0;
static int S_INT                  = 0;
static int S_MENU                 = 2;
static int S_SIMULATION           = 4;
static int S_CREDITS              = 8;







// FUNCTIONS LIST
// ----------------------------------------------------------------------------

void display();
void render_loop();
void reshape();
void setup_lighting();
void setup_gl();
void setup_scene();
void camera_move();
void draw_floor();
void fullscreen_toggle();
void mouse();
void special_keys();

void arena_setup();
void arena_draw();
void arena_loop();
void arena_player_loop();
bool arena_player_move();
void arena_player_rotate();
void arena_draw_terrain();
void arena_flag_terrain();
void arena_draw_player_model();
void arena_draw_entities();














// MATH HELPERS
// ----------------------------------------------------------------------------



float random_f(){
   float r = (float)rand() / (float)RAND_MAX;
   return r;
}

float random_fcolor(){
   float r = random_f();
   if (r < RANDOM_MIN_COLOUR){
      r = RANDOM_MIN_COLOUR;
   }
   if  (r > RANDOM_MAX_COLOUR){
      r = RANDOM_MAX_COLOUR;
   }
   return r;
}





















// ARENA
// ----------------------------------------------------------------------------


void arena_spawn_entity(int ai){
  int free_id = 0;

  for (int i = 1; i < ENTITIES_SIZE_MAX; i++){
    if (entities[i][3] == 0.0f){
      free_id = i;
      break;
    }
  }

  if (free_id > 0){
    entities[free_id][0] = 4+(float)(int)(random_f() * (CELLS_ARRAY_SIZE[0]-8));
    entities[free_id][1] = 4+(float)(int)(random_f() * (CELLS_ARRAY_SIZE[1]-8));
    entities[free_id][2] = (float)(int)(random_f() * 4);
    entities[free_id][3] = 1.0f;
    entities_rotated[free_id] = false;
  }
}

void arena_setup(){
  for (int y = 0; y < CELLS_ARRAY_SIZE[1]; y++){
  for (int x = 0; x < CELLS_ARRAY_SIZE[0]; x++){
    if (x == 0  or x == CELLS_ARRAY_SIZE[0]-1  or y == 0 or y == CELLS_ARRAY_SIZE[1] -1 ){
      arena_array[x][y] = WALL_MAX;
    }else{
      arena_array[x][y] = GROUND;
    }

  }}

  for (int i = 1; i < ENTITIES_SIZE_MAX; i++){
    if (i < ENTITIES_SIZE_START){
      arena_spawn_entity(0);
    }else{
      entities[i][0] = 0.0f;
      entities[i][1] = 0.0f;
      entities[i][2] = 0;
      entities[i][3] = 0.0f;
      entities_rotated[i] = false;
    }


  }
}

void arena_draw_terrain_tile(float x, float y, float z){
  glPushMatrix();

      glTranslatef (x, y, 0.0f);

    glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
    glEnable ( GL_COLOR_MATERIAL ) ;
    float color[] = {z, z, z, 1.0f};
    color[0] += 0.2;
    color[1] += 0.2;
    color[2] += 0.2;
    if (z >= WALL){
      color[2] = 0.9f;
    }
    glColor4f(color[0], color[1], color[2], 1.0f);

    float size = 0.5f;

    glBegin(GL_POLYGON);
       glVertex3d(-size, size, 0.0f);
       glVertex3d(-size, -size, 0.0f);
       glVertex3d(size, -size, 0.0f);
       glVertex3d(size, size, 0.0f);
    glEnd();
    glPopMatrix();
}

void  arena_draw_terrain(){
  float tile;

  for (int y = 0; y < CELLS_ARRAY_SIZE[1]; y++){
  for (int x = 0; x < CELLS_ARRAY_SIZE[0]; x++){
    tile = arena_array[x][y];
    arena_draw_terrain_tile(x, y, tile);
  }}
}

void arena_draw_player_model(int e){
  int x = entities[e][0];
  int y = entities[e][1];
  int rot = entities[e][2];
  int health = entities[e][3];
  float terrain = arena_array[x][y];
  float size = 0.5f;

  glPushMatrix();
    glTranslatef ((float)x, (float)y, 0.1f);
    glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
    glEnable ( GL_COLOR_MATERIAL ) ;

    if(e==0){
      glColor4f(0.4f, 0.4f, 1.0f, 1.0f);
    }else{
      glColor4f(1.0f, e % 2 == 0 ? 0.0f : 1.0f, 0.4f, 1.0f);
    }

    glBegin(GL_POLYGON);
      switch(rot){
        case 0:
         glVertex3d(-size, 0, 0.0f);
         glVertex3d(0, size, 0.0f);
         glVertex3d(size, 0, 0.0f);
        break;
        case 1:
         glVertex3d(size, 0, 0.0f);
         glVertex3d(0, -size, 0.0f);
         glVertex3d(0, size, 0.0f);
        break;
        case 2:
         glVertex3d(0, -size, 0.0f);
         glVertex3d(size, 0, 0.0f);
         glVertex3d(-size, 0, 0.0f);
        break;
        case 3:
         glVertex3d(-size, 0, 0.0f);
         glVertex3d(0, size, 0.0f);
         glVertex3d(0, -size, 0.0f);
        break;
      }
    glEnd();
  glPopMatrix();
}

void  arena_draw_entities(){
  float player[3];

  for (int i = 0; i < ENTITIES_SIZE_MAX; i++){
    arena_draw_player_model(i);
  }
}


void arena_expand_terrain(){
  float tile;

  for (int y = 0; y < CELLS_ARRAY_SIZE[1]; y++){
  for (int x = 0; x < CELLS_ARRAY_SIZE[0]; x++){
    tile = arena_array[x][y];
    if (tile >= WALL and tile < WALL_MAX){
       arena_array[x][y] += EXPAND;
    }
  }}
}

void arena_terrain_lower(int p){

  int x = entities[p][0];
  int y = entities[p][1];

  if (arena_array[x][y] > 0.0f){
    if(p == 0){
      arena_array[x][y] -= LOWER_P;
    }else{
      arena_array[x][y] -= LOWER;
    }
  }
}

void arena_damage_wall(int x, int y){
  if (WALL_CAN_BE_DESTROYED){
  if (arena_array[x][y] > 0.0f){
  if (x != 0 and y != 0){
  if (x < CELLS_ARRAY_SIZE[0]-1 and y < CELLS_ARRAY_SIZE[1]-1){
    arena_array[x][y] -= LOWER_P;
  }}}}
}

bool arena_player_move_check(int p, bool move){
  int x = entities[p][0];
  int y = entities[p][1];
  int rotate = entities[p][2];

  switch (rotate) {
    case 0:
      y++;
    break;
    case 1:
      x++;
    break;
    case 2:
      y--;
    break;
    case 3:
      x--;
    break;
  }

  if (arena_array[x][y] < WALL){
    if (move){
      entities[p][0] = x;
      entities[p][1] = y;
    }

    return true;
  }else{
    arena_damage_wall(x, y);
    return false;
  }
}

void arena_player_move(int p, int rotate){
  if (arena_player_move_check(p, true)){
    arena_terrain_lower(p);
  }
}

void arena_player_rotate(int p, int dir){
  if (entities[p][2] != dir){
    entities[p][2] = dir;
    entities_rotated[p] = true;
  }else{
    entities_rotated[p] = false;
  }
  if (p == 0) arena_player_loop();
  arena_loop();
}

void arena_flag_terrain(int p){
  int x = entities[p][0];
  int y = entities[p][1];
  if (arena_player_move_check(p, false)){
    arena_array[x][y] = WALL;
    if (p == 0) arena_player_loop();
    arena_loop();
  }
}

void arena_cam_follow_player(int p){
  switch(cam_mode){
    case 0:
      cam_pos[0] = entities[0][0];
      cam_pos[1] = entities[0][1]-14.0f;
      cam_pos[2] = 14.0f;
      cam_look_pos[0] = entities[0][0];
      cam_look_pos[1] = entities[0][1];
      cam_look_pos[2] = 4.0f;
    break;

    case 1:
      cam_pos[0] = entities[0][0];
      cam_pos[1] = entities[0][1];
      cam_pos[2] = 14.0f;
      cam_look_pos[0] = entities[0][0];
      cam_look_pos[1] = entities[0][1];
      cam_look_pos[2] = 4.0f;
    break;

    case 2:
      cam_pos[0] = entities[0][0];
      cam_pos[1] = entities[0][1];
      cam_pos[2] = 26.0f;
      cam_look_pos[0] = entities[0][0];
      cam_look_pos[1] = entities[0][1];
      cam_look_pos[2] = 4.0f;
    break;
  }

}


void arena_ai_simple(int e){
  if( random_f() > 0.8f){
    entities[e][2] = (float)(int)(random_f() * 4);
    entities_rotated[e] = true;
  }
}

void arena_ai_medium(int e){
  int x = entities[e][0];
  int y = entities[e][1];
  int rot = entities[e][2];
  float candids[] = {arena_array[x][y+1], arena_array[x+1][y], arena_array[x][y-1], arena_array[x-1][y]};
  int candid_rots[] = {rot,rot,rot,rot};
  int a = 0;
  int r = rot;
  float treshold = 0.15f;

  for (int i = 0; i < 4; i++){
    if(candids[rot] - candids[i] >= treshold){
      candid_rots[a++] = i;
    }
  }

    r = (int)(random_f() * a);
    if (rot != candid_rots[r]){
      entities[e][2] = candid_rots[r];
      entities_rotated[e] = true;
    }
}

void  arena_draw(){
  arena_draw_terrain();
  arena_draw_entities();
  arena_expand_terrain();
}

void arena_loop(){
  for (int i = 1; i < ENTITIES_SIZE_MAX; i++){
    if( entities[i][3] > 0.0f){
      if(i>0){
        if (i % 2 == 0){
          arena_ai_simple(i);
        }else{
          arena_ai_medium(i);
        }
      }

      if(!entities_rotated[i]){
        arena_player_move(i, entities[i][2]);
      }else{
        entities_rotated[i] = false;
      }
    }
  }
  arena_cam_follow_player(0);
}

void arena_player_loop(){
  if(!entities_rotated[0]){
    arena_player_move(0, entities[0][2]);
  }else{
    entities_rotated[0] = false;
  }
}


// INPUTS
// ----------------------------------------------------------------------------

void mouse(int button, int state, int x, int y) {
  // void
}

void special_keys(int key, int x, int y) {
   switch (key) {
      case GLUT_KEY_F1:
         fullscreen_mode = !fullscreen_mode;
         fullscreen_toggle();
         break;
      case GLUT_KEY_RIGHT:
        arena_player_rotate(0, 1);
        break;
      case GLUT_KEY_LEFT:
        arena_player_rotate(0, 3);
        break;
      case GLUT_KEY_UP:
        arena_player_rotate(0, 0);
        break;
      case GLUT_KEY_DOWN:
        arena_player_rotate(0, 2);
        break;
      case GLUT_KEY_F2:
        arena_spawn_entity(0);
        break;
      case GLUT_KEY_F3:
        cam_mode = 0;
        break;
      case GLUT_KEY_F4:
        cam_mode = 1;
        break;
      case GLUT_KEY_F5:
        cam_mode = 2;
        break;

   }
}

void keyboard(unsigned char key, int x, int y) {
   switch (key) {
      case 27:     // ESC key
         //exit(0);
         break;
      case 13: // enter
          arena_flag_terrain(0);
         break;
      case 113: // q

        break;
      case 101: // e

        break;
      case 97: // a

        break;
      case 100: // d

        break;

      case 119: // w

        break;
      case 115: // s

        break;
   }
}









// OPEN GL
// ----------------------------------------------------------------------------

void reshape(GLsizei width, GLsizei height) {
  if (width == 0) width = 12;
  if (height == 0) height = 12;

  glViewport(0, 0, width, height);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  gluPerspective (FOV, (GLfloat)width/(GLfloat)height, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void fullscreen_toggle(){
  if (fullscreen_mode) {
    glutFullScreen();
  } else {
    glutReshapeWindow(win_width, win_height);
    glutPositionWindow(win_x, win_y);
  }
}

void setup_lighting(){
  float amb = 0.6f;
  float diff = 0.3f;
  float spec = 0.2f;

  GLfloat global_ambient[]  = {amb, amb, amb, 0.2f};
  GLfloat light_ambient[]   = {amb, amb, amb, 1.0f };
  GLfloat light_diffuse[]   = {diff, diff, diff, 1.0f };
  GLfloat light_specular[]  = {spec, spec, spec, 1.0f };
  GLfloat light_position[]  = {0.0f, 0.0f, 0.0f, 0.0f };

  glEnable(GL_LIGHTING);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHT0);

  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogfv(GL_FOG_COLOR, cam_clear_color);
  glFogf(GL_FOG_START, cam_fog_start);
  glFogf(GL_FOG_END, cam_fog_end);
  glEnable(GL_FOG);
}

void setup_gl(){
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glShadeModel(GL_SMOOTH);
  //glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void setup_app() {
  glutCreateWindow(win_title);
  fullscreen_toggle();
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutSpecialFunc(special_keys);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
}

void setup_menu(){

}

void camera_move(){
  glLoadIdentity();
  float treshold = 0.05f;
  for (int i = 0; i < 3; i++){
    if(cam_pos[i+3] > cam_pos[i]){
      cam_pos[i+3] -= cam_move_speed;
    }
    if(cam_pos[i+3] < cam_pos[i]){
      cam_pos[i+3] += cam_move_speed;
    }
  }

  for (int i = 0; i < 3; i++){
    if(cam_look_pos[i+3] > cam_look_pos[i]){
      cam_look_pos[i+3] -= cam_look_move_speed;
    }
    if(cam_look_pos[i+3] < cam_look_pos[i]){
      cam_look_pos[i+3] += cam_look_move_speed;
    }
  }


  gluLookAt(cam_pos[3], cam_pos[4], cam_pos[5],
    cam_look_pos[3], cam_look_pos[4], cam_look_pos[5],
    0.0, 1.0, 0.0);
}

void setup_scene(){
  setup_lighting();
  STATE = S_SIMULATION;
}














// MAIN LOOPS
// ----------------------------------------------------------------------------

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glClearColor(0.3f, 0.05f, 0.6f, 1.0f);

  glLoadIdentity();
  camera_move();

  if (STATE == S_SIMULATION){
    arena_draw();
  }

  glFlush();
  glutSwapBuffers();
}

void render_loop(int value) {
  glutPostRedisplay();
  glutTimerFunc(refresh_ms, render_loop, 0);
}

void logic_loop(int value){
  if (REALTIME){
    arena_loop();
  }
  glutTimerFunc(LOGIC_FPS, logic_loop, 0);
}

int main(int argc, char** argv) {
  glutInit(&argc, argv);
  setup_app();
  setup_menu();
  setup_scene();
  arena_setup();
  glutTimerFunc(0, render_loop, 0);
  glutTimerFunc(0, logic_loop, 0);
  glutMainLoop();
   return 0;
}



// The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.