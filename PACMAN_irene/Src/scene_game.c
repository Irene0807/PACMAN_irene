#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "shared.h"
#include "utility.h"
#include "scene_game.h"
#include "scene_menu.h"
#include "scene_gameover.h"
#include "scene_win.h"
#include "scene_win.h"
#include "pacman_obj.h"
#include "ghost.h"
#include "map.h"


// [HACKATHON 2-0]
// Just modify the GHOST_NUM to 1
#define GHOST_NUM 4
/* global variables*/
extern const uint32_t GAME_TICK_CD;
extern uint32_t GAME_TICK;
extern ALLEGRO_TIMER* game_tick_timer;
extern ALLEGRO_TIMER* power_up_timer;
extern ALLEGRO_TIMER* flee_timer;
extern ALLEGRO_TIMER* go_out_timer;
extern ALLEGRO_TIMER* speedup_timer;
extern ALLEGRO_TIMER* penetrate_timer;
extern int game_main_Score = 0;	
extern int flag = 0;
extern int map_i;
bool ispenetrate = false;
bool isspeedup = false;
bool game_over = false;
bool near_powerdown = false;

/* Internal variables*/

static const int duration = 10;
static Pacman* pman;
static Map* basic_map;
static Ghost** ghosts;
static ALLEGRO_SAMPLE_ID inBGM;
static ALLEGRO_SAMPLE_ID PACMAN_EATSOUND;
bool debug_mode = false;
bool cheat_mode = false;
bool win = false;
char prscore[3000];
char prtime[3000];

/* Declare static function prototypes */
static void init(void);
static void step(void);
static void checkItem(void);
static void status_update(void);
static void update(void);
static void draw(void);
static void printinfo(void);
static void destroy(void);
static void on_key_down(int key_code);
static void on_mouse_down(void);
static void render_init_screen(void);
static void draw_hitboxes(void);

void powerup(void);
void powerdown(void);
void inspeedup(void);
void unspeedup(void);
void inpenetrate(void);
void unpenetrate(void);


static void init(void) {
	game_over = false;
	game_main_Score = 0;
	
	// [TODO]
	// Create map from .txt file and design your own map !!
	if(map_i == 1)	basic_map = create_map("Assets/map_nthu.txt");
	else if(map_i == 2) basic_map = create_map("Assets/map_I2PYANG.txt");
	else if(map_i == 3) basic_map = create_map("Assets/map_mustwin.txt");
	if (!basic_map) {
		game_abort("error on creating map");
	}	
	// create pacman
	pman = pacman_create();
	if (!pman) {
		game_abort("error on creating pacamn\n");
	}
	
	// allocate ghost memory
	// [HACKATHON 2-1]
	// TODO: Allocate dynamic memory for ghosts array.
	
	ghosts = (Ghost**)malloc(sizeof(Ghost*) * GHOST_NUM);
	
	if(!ghosts){
		game_log("We haven't create any ghosts!\n");
	}
	else {
		// [HACKATHON 2-2]
		// TODO: create a ghost.
		// Try to look the definition of ghost_create and figure out what should be placed here.
		for (int i = 0; i < GHOST_NUM; i++) {
			
			game_log("creating ghost %d\n", i);
			if(i == 0) ghosts[i] = ghost_create(Blinky);  
			else if(i == 1) ghosts[i] = ghost_create(Clyde);
			else if (i == 2) ghosts[i] = ghost_create(Inky);
			else if (i == 3) ghosts[i] = ghost_create(Pinky);
			if (!ghosts[i])
				game_abort("error creating ghost\n");
			
		}
	}
	GAME_TICK = 0;
	inBGM = play_bgm(inMusic, music_volume + 1);
	render_init_screen();
	power_up_timer = al_create_timer(1.0f); // 1 tick / sec
	if (!power_up_timer)
		game_abort("Error on create timer\n");
	return ;
}

static void step(void) {
	if (pman->objData.moveCD > 0)
		pman->objData.moveCD -= pman->speed;
	for (int i = 0; i < GHOST_NUM; i++) {
		// important for movement
		if (ghosts[i]->objData.moveCD > 0)
			ghosts[i]->objData.moveCD -= ghosts[i]->speed;
	}
}
static void checkItem(void) {
	int Grid_x = pman->objData.Coord.x, Grid_y = pman->objData.Coord.y;
	if (Grid_y >= basic_map->row_num - 1 || Grid_y <= 0 || Grid_x >= basic_map->col_num - 1 || Grid_x <= 0)
		return;
	// [HACKATHON 1-3]
	// TODO: check which item you are going to eat and use `pacman_eatItem` to deal with it.
	
	switch (basic_map->map[Grid_y][Grid_x])
	{
	case '.':
		basic_map->beansCount--;
		game_main_Score += 10;
		pacman_eatItem(pman, basic_map->map[Grid_y][Grid_x]);
		break;
	case 'P':
		game_main_Score += 50;
		powerup();
		break;
	case 'S':
		inspeedup();
		break;
	case 'E':
		inpenetrate();
		break;
	default:
		break;
	}
	
	// [HACKATHON 1-4]
	// erase the item you eat from map
	// be careful no erasing the wall block.
	////if(basic_map->map[Grid_y][Grid_x] != '#')
		basic_map->map[Grid_y][Grid_x] = 0;
	
}
static void status_update(void) {
	for (int i = 0; i < GHOST_NUM; i++) {
		if (ghosts[i]->status == GO_IN)
			continue;
		// [TODO]
		// use `getDrawArea(..., GAME_TICK_CD)` and `RecAreaOverlap(..., GAME_TICK_CD)` functions to detect
		// if pacman and ghosts collide with each other.
		// And perform corresponding operations.
		// [NOTE]
		// You should have some branch here if you want to implement power bean mode.
		// Uncomment Following Code
		
		if(!cheat_mode && RecAreaOverlap( getDrawArea(pman->objData, GAME_TICK_CD) , getDrawArea(ghosts[i]->objData, GAME_TICK_CD) )) {
			game_log("collide with ghost\n");

			if (pman->powerUp && ghosts[i]->status == FLEE)
			{
				game_main_Score += 200;
				ghost_collided(ghosts[i]);
				al_start_timer(flee_timer);
				al_set_timer_count(flee_timer, 0);
				continue;
			}

			al_rest(1.0);
			pacman_die();
			game_over = true;
			break;
		}
		if (basic_map->beansCount == 0)
		{
			game_log("ate all beans\n");

			win = true;
			
			if (debug_mode) {
				draw_hitboxes();
			}
			al_flip_display();
			break;
		}
		
	}
}

static void update(void) {

	if (game_over) {
		
		//[TODO]
		//start pman->death_anim_counter and schedule a game-over event (e.g change scene to menu) after Pacman's death animation finished
		stop_bgm(inBGM);
		stop_bgm(PACMAN_EATSOUND);
		al_start_timer(pman->death_anim_counter);

		if (al_get_timer_count(pman->death_anim_counter) == 17 * 8) {
			isspeedup = false;
			ispenetrate = false;
			al_set_timer_count(speedup_timer, 0);
			al_stop_timer(speedup_timer);
			al_set_timer_count(penetrate_timer, 0);
			al_stop_timer(penetrate_timer);
			game_change_scene(scene_gameover_create());
		}
		return;
	}
	if (win) {
		stop_bgm(inBGM);
		stop_bgm(PACMAN_EATSOUND);
		isspeedup = false;
		ispenetrate = false;
		al_set_timer_count(speedup_timer, 0);
		al_stop_timer(speedup_timer);
		al_set_timer_count(penetrate_timer, 0);
		al_stop_timer(penetrate_timer);
		game_change_scene(scene_gameover_create());
		al_rest(1.0);
		game_change_scene(scene_win_create());
		win = false;
		return;

	}
	if (al_get_timer_count(power_up_timer) == duration - 3)
		near_powerdown = true;
	if (al_get_timer_count(power_up_timer) == duration)
		powerdown();
	if (al_get_timer_count(speedup_timer) == 5)
		unspeedup();
	if (al_get_timer_count(penetrate_timer) == 5)
		unpenetrate();

	if (al_get_timer_count(flee_timer) == 5)
	{
		//stop_bgm(pointBGM);
		al_set_timer_count(flee_timer, 0);
		al_stop_timer(flee_timer);
	}

	step();
	checkItem();
	status_update();
	pacman_move(pman, basic_map);
	for (int i = 0; i < GHOST_NUM; i++) 
		ghosts[i]->move_script(ghosts[i], basic_map, pman);
}

static void draw(void) {

	al_clear_to_color(al_map_rgb(0, 0, 0));

	//	[TODO]
	//	Draw scoreboard, something your may need is sprinf();
	snprintf(prscore, 1000, "SCORE: %d", game_main_Score);
	snprintf(prtime, 1000, "TIME: %d", al_get_timer_count(game_tick_timer) / 100);
	al_draw_text(menuFont, al_map_rgb(230, 230, 255), 45, 20, 0, prscore);
	al_draw_text(menuFont, al_map_rgb(230, 230, 255), 640, 20, 0, prtime);

	draw_map(basic_map);
	pacman_draw(pman);
	

	if (game_over)
		return;
	// no drawing below (ghosts) when game over
	for (int i = 0; i < GHOST_NUM; i++)
		ghost_draw(ghosts[i]);
	
	//debugging mode
	if (debug_mode) {
		draw_hitboxes();
	}

}

static void draw_hitboxes(void) {
	RecArea pmanHB = getDrawArea(pman->objData, GAME_TICK_CD);
	al_draw_rectangle(
		pmanHB.x, pmanHB.y,
		pmanHB.x + pmanHB.w, pmanHB.y + pmanHB.h,
		al_map_rgb_f(1.0, 0.0, 0.0), 2
	);

	for (int i = 0; i < GHOST_NUM; i++) {
		RecArea ghostHB = getDrawArea(ghosts[i]->objData, GAME_TICK_CD);
		al_draw_rectangle(
			ghostHB.x, ghostHB.y,
			ghostHB.x + ghostHB.w, ghostHB.y + ghostHB.h,
			al_map_rgb_f(1.0, 0.0, 0.0), 2
		);
	}

}

static void printinfo(void) {
	game_log("pacman:\n");
	game_log("coord: %d, %d\n", pman->objData.Coord.x, pman->objData.Coord.y);
	game_log("PreMove: %d\n", pman->objData.preMove);
	game_log("NextTryMove: %d\n", pman->objData.nextTryMove);
	game_log("Speed: %f\n", pman->speed);
}


static void destroy(void) {
	
		//[TODO]
		//free map array, Pacman and ghosts
		if(basic_map){
			for (int i = 0; i < basic_map->row_num; i++) 
				free(basic_map->map[i]);
			free(basic_map->map);

		}
		free(basic_map);
		free(pman);
		for (int i = 0; i<GHOST_NUM; i++)
			free(ghosts[i]);
		free(ghosts);
		stop_bgm(inBGM);
		stop_bgm(PACMAN_EATSOUND);

}

static void on_key_down(int key_code) {

	if (flag ==1) {

		switch (key_code){
			// [HACKATHON 1-1]	
			// TODO: Use allegro pre-defined enum ALLEGRO_KEY_<KEYNAME> to controll pacman movement
			// we provided you a function `pacman_NextMove` to set the pacman's next move direction.

		case ALLEGRO_KEY_UP:
			pacman_NextMove(pman, UP);
			break;
		case ALLEGRO_KEY_LEFT:
			pacman_NextMove(pman, LEFT);
			break;
		case ALLEGRO_KEY_DOWN:
			pacman_NextMove(pman, DOWN);
			break;
		case ALLEGRO_KEY_RIGHT:
			pacman_NextMove(pman, RIGHT);
			break;
		case ALLEGRO_KEY_RSHIFT:
			cheat_mode = !cheat_mode;
			if (cheat_mode)
				printf("cheat mode on\n");
			else
				printf("cheat mode off\n");
			break;
		case ALLEGRO_KEY_RCTRL:
			debug_mode = !debug_mode;
			break;
		default:
			break;
		}
	}
	else if(flag == 0) {

		switch (key_code) {
			// [HACKATHON 1-1]	
			// TODO: Use allegro pre-defined enum ALLEGRO_KEY_<KEYNAME> to controll pacman movement
			// we provided you a function `pacman_NextMove` to set the pacman's next move direction.

		case ALLEGRO_KEY_W:
			pacman_NextMove(pman, UP);
			break;
		case ALLEGRO_KEY_A:
			pacman_NextMove(pman, LEFT);
			break;
		case ALLEGRO_KEY_S:
			pacman_NextMove(pman, DOWN);
			break;
		case ALLEGRO_KEY_D:
			pacman_NextMove(pman, RIGHT);
			break;
		case ALLEGRO_KEY_C:
			cheat_mode = !cheat_mode;
			if (cheat_mode)
				printf("cheat mode on\n");
			else
				printf("cheat mode off\n");
			break;
		case ALLEGRO_KEY_G:
			debug_mode = !debug_mode;
			break;
		default:
			break;
		}
	}

}

static void on_mouse_down(void) {
	// nothing here

}

static void render_init_screen(void) {
	al_clear_to_color(al_map_rgb(0, 0, 0));

	draw_map(basic_map);
	pacman_draw(pman);
	for (int i = 0; i < GHOST_NUM; i++) {
		ghost_draw(ghosts[i]);
	}

	al_draw_text(menuFont, al_map_rgb(255, 255, 0), 400, 400, ALLEGRO_ALIGN_CENTER, "READY!");
	al_flip_display();
	al_rest(2.0);


}
// Functions without 'static', 'extern' prefixes is just a normal
// function, they can be accessed by other files using 'extern'.
// Define your normal function prototypes below.

void powerup(void)
{
	al_start_timer(power_up_timer);
	al_set_timer_count(power_up_timer, 0);
	pman->powerUp = true;
	for (int i = 0; i < GHOST_NUM; i++)
	{
		ghost_toggle_FLEE(ghosts[i], 1, basic_map);
	}
	GAME_TICK = 1;
	near_powerdown = false;
	return;
}

void powerdown(void)
{
	for (int i = 0; i < GHOST_NUM; i++)
	{
		ghost_toggle_FLEE(ghosts[i], 0, basic_map);
	}
	pman->powerUp = false;
	al_set_timer_count(power_up_timer, 0);
	al_stop_timer(power_up_timer);
	return;
}

void inspeedup(void)
{
	isspeedup = true;
	al_start_timer(speedup_timer);
	al_set_timer_count(speedup_timer, 0);
	stop_bgm(PACMAN_EATSOUND);
	pman->speed = 4;

}

void unspeedup(void)
{

	isspeedup = false;
	pman->speed = 2;
	al_set_timer_count(speedup_timer, 0);
	al_stop_timer(speedup_timer);
	stop_bgm(PACMAN_EATSOUND);

}

void inpenetrate(void)
{
	ispenetrate = true;
	al_start_timer(penetrate_timer);
	al_set_timer_count(penetrate_timer, 0);

}

void unpenetrate(void)
{
	ispenetrate = false;
	al_set_timer_count(penetrate_timer, 0);
	al_stop_timer(penetrate_timer);
	stop_bgm(PACMAN_EATSOUND);

}



// The only function that is shared across files.
Scene scene_main_create(void) {
	Scene scene;
	memset(&scene, 0, sizeof(Scene));
	scene.name = "Start";
	scene.initialize = &init;
	scene.update = &update;
	scene.draw = &draw;
	scene.destroy = &destroy;
	scene.on_key_down = &on_key_down;
	scene.on_mouse_down = &on_mouse_down;
	// TODO: Register more event callback functions such as keyboard, mouse, ...
	game_log("Start scene created");
	return scene;
}
