#include "pebble.h"

#define GRID_WIDTH 4
#define TILE_SIZE 34
#define SPACING_SIZE 2
#define ACCEL_THRESHOLD 100
#define SPAWN_TIMER 500
#define SCORE_LIMIT 128
#define FAST 0
#define SLOW 1

static int _UP = 0, _RIGHT = 1, _DOWN = 2, _LEFT = 3;


typedef struct Tile {
	int value;
	GRect rect;	
} Tile;

static Tile tiles[GRID_WIDTH][GRID_WIDTH];
static Window *window;
static GRect window_frame;
static Layer *tile_layer;
static AppTimer *timer;
static int count = 0;
static int dir = -1;
static bool is_winning = false;
static int gamemode = SLOW;

static void tile_set_value(Tile *tile, int val) {
	tile->value = val;
}
static void tile_init(int x, int y, int val) {
	Tile *tile = &tiles[x][y]; 
	tile->rect.origin.x = x * TILE_SIZE + x * SPACING_SIZE;
	tile->rect.origin.y = y * TILE_SIZE + y * SPACING_SIZE;
	
	tile->rect.size.w = TILE_SIZE;
	tile->rect.size.h = TILE_SIZE;
	
	tile_set_value(tile,val);
}

static void tile_draw(GContext *ctx, Tile *tile) {
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, tile->rect, 3, GCornersAll);
	char str[4];
	snprintf(str, sizeof(str), "%d", tile->value);
	const char* val = str;
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(
		ctx,
		val,
		fonts_get_system_font(FONT_KEY_GOTHIC_14),
		tile->rect,
		GTextOverflowModeFill,
		GTextAlignmentCenter,
		NULL);	
}
static void tile_draw_text(GContext *ctx, int x, int y, const char* val){
	Tile *tile = &tiles[x][y];
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_fill_rect(ctx, tile->rect, 3, GCornersAll);
	graphics_context_set_text_color(ctx, GColorBlack);
	graphics_draw_text(
		ctx,
		val,
		fonts_get_system_font(FONT_KEY_GOTHIC_14),
		tile->rect,
		GTextOverflowModeFill,
		GTextAlignmentCenter,
		NULL);
}
static void victory_draw(GContext *ctx) {
	char* you = "YOU";
	char* win = "WIN";
	tile_draw_text(ctx, 1, 1, you);
	tile_draw_text(ctx, 2, 1, win);
}

static void win(GContext *ctx){
	victory_draw(ctx);
	timer = NULL;
}

static bool tile_is_open(Tile *tile){
	return tile->value < 0;
}
static void tile_layer_update_callback(Layer *me, GContext *ctx) {
	for (int i = 0; i < GRID_WIDTH; i++) {
		for (int j = 0; j < GRID_WIDTH; j++) {
			if(is_winning){
				win(ctx);
			}else if(!tile_is_open(&tiles[i][j]))
				tile_draw(ctx, &tiles[i][j]);
		}
	}
}

static float* get_average(AccelData *data, uint32_t num_samples){
	static float avg[2];
	float x = data[0].x, y = data[0].y;
	for(int i = 1; i < (int)num_samples; i++){
		x += data[i].x;
		y += data[i].y;
	}
	avg[0] = x/num_samples;
	avg[1] = y/num_samples;
	return avg;
}

static void shift_tile(Tile *origin, Tile *dest){
	tile_set_value(dest, origin->value);
	tile_set_value(origin, -1);
}

/*
 * Fills available whitespace in the direction of dir
 */
static void mind_the_gap(int dir){

	switch(dir){
	case 0:		// up
		for (int i = 0; i < GRID_WIDTH; i++) {
			for (int j = 1; j < GRID_WIDTH; j++) {
				Tile *tile = &tiles[i][j];
				Tile *neighbor = &tiles[i][j-1];
				if(tile_is_open(neighbor))
					shift_tile(tile, neighbor);
			}
		}
		break;
		
	case 1:		// right
		for (int i = GRID_WIDTH - 2; i >= 0; i--) {
			for (int j = 0; j < GRID_WIDTH; j++) {
				Tile *tile = &tiles[i][j];
				Tile *neighbor = &tiles[i+1][j];
				if(tile_is_open(neighbor))
					shift_tile(tile, neighbor);
			}
		}
		break;
		
	case 2:		// down
		for (int i = 0; i < GRID_WIDTH; i++) {
			for (int j = GRID_WIDTH - 2; j >= 0; j--) {
				Tile *tile = &tiles[i][j];
				Tile *neighbor = &tiles[i][j+1];
				if(tile_is_open(neighbor))
					shift_tile(tile, neighbor);
			}
		}
		break;
		
	case 3:		// left
		for (int i = 1; i < GRID_WIDTH; i++) {
			for (int j = 0; j < GRID_WIDTH; j++) {
				Tile *tile = &tiles[i][j];
				Tile *neighbor = &tiles[i-1][j];
				if(tile_is_open(neighbor))
					shift_tile(tile, neighbor);
			}
		}
		break;
	}
}
static void combine_tiles(Tile *origin, Tile *dest){
	if(origin->value == dest->value){				
		tile_set_value(dest, origin->value + dest->value);
		tile_set_value(origin, -1);
		if(origin->value + dest->value >= SCORE_LIMIT)
			is_winning = true;
	}
}
/*
 * Combines neighboring tiles if they have like values
 */
static void combine_neighbors(int dir){
	switch(dir){
	case 0:		// up
		for (int i = 0; i < GRID_WIDTH; i++) {
			for (int j = 1; j < GRID_WIDTH; j++) {
				Tile *tile = &tiles[i][j];
				Tile *neighbor = &tiles[i][j-1];
				combine_tiles(tile, neighbor);
			}
		}
		break;
		
	case 1:		// right
		for (int i = GRID_WIDTH - 2; i >= 0; i--) {
			for (int j = 0; j < GRID_WIDTH; j++) {
				Tile *tile = &tiles[i][j];
				Tile *neighbor = &tiles[i+1][j];
				combine_tiles(tile, neighbor);
			}
		}
		break;
		
	case 2:		// down
		for (int i = 0; i < GRID_WIDTH; i++) {
			for (int j = GRID_WIDTH - 2; j >= 0; j--) {
				Tile *tile = &tiles[i][j];
				Tile *neighbor = &tiles[i][j+1];
				combine_tiles(tile, neighbor);
			}
		}
		break;
		
	case 3:		// left
		for (int i = 1; i < GRID_WIDTH; i++) {
			for (int j = 0; j < GRID_WIDTH; j++) {
				Tile *tile = &tiles[i][j];
				Tile *neighbor = &tiles[i-1][j];
				combine_tiles(tile, neighbor);
			}
		}
		break;
	}
}
/*
 * Shifts all tiles in the direction of dir and combines like neighbors
 */
static void shift_grid(int dir){
	mind_the_gap(dir);
	mind_the_gap(dir);
	mind_the_gap(dir);
	combine_neighbors(dir);
}
/*
 *	Determines the direction of the accelerometer
 */
static void accel_handler(AccelData *data, uint32_t num_samples){
	float* avg = get_average(data, num_samples);
	float x = avg[0];
	float y = avg[1];
	if (x > 0 + ACCEL_THRESHOLD){
    	if(y > 0 + ACCEL_THRESHOLD){
    		if(x > y)
    			dir = _RIGHT;
    		else if(x < y)
    			dir = _UP;
    		else
  				dir = -1;
    	}else if(y < 0 - ACCEL_THRESHOLD){
    		if(x > -1 * y)
    			dir = _RIGHT;
    		else if(x < -1 * y)
    			dir = _DOWN;
    		else
  				dir = -1;
    	}
    	else
  			dir = -1;
  	} else if (x < 0 - ACCEL_THRESHOLD){
    	if(y > 0 + ACCEL_THRESHOLD){
    		if(x * -1 > y)
    			dir = _LEFT;
    		else if(x * -1 < y)
    			dir = _UP;
    		else
  				dir = -1;
    	}else if(y < 0 - ACCEL_THRESHOLD){
    		if(x * -1 > -1 * y)
    			dir = _LEFT;
    		else if(x * -1 < -1 * y)
    			dir = _DOWN;
    		else
  				dir = -1;
    	}
    	else
  			dir = -1;
  	} else {
  		dir = -1;
  	}
  	
	if(dir > -1)
		shift_grid(dir);
}
static int* find_open_cell(){
	bool open = false;
	int x = -1, y = -1;
	int num_tries = 0;
	while(!open && num_tries < GRID_WIDTH * GRID_WIDTH * GRID_WIDTH){
		x = rand() % GRID_WIDTH;
		y = rand() % GRID_WIDTH;
		if(tile_is_open(&tiles[x][y]))
			open = true;
		num_tries++;
	}
	static int arr[2];
	arr[0] = x;
	arr[1] = y;
	return arr;
}
static void random_tile(){
	int* pos = find_open_cell();
	int x = pos[0];
	int y = pos[1];
	Tile *tile = &tiles[x][y];
	tile_set_value(tile,2);
	count++;
}
static void timer_callback(void *data) {
	if(dir > -1){
		if(gamemode == FAST) random_tile();
	}
	
	layer_mark_dirty(tile_layer);

	timer = app_timer_register(SPAWN_TIMER, timer_callback, NULL);
}
static void init_grid(){
	for(int i = 0; i < GRID_WIDTH; i++){
		for(int j = 0; j < GRID_WIDTH; j++){
			tile_init(i,j,-1);
		}
	}
 	random_tile();
 	random_tile();
}
static void init_game(){
	init_grid();
	timer = app_timer_register(SPAWN_TIMER, timer_callback, NULL);
}
static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect frame = window_frame = layer_get_frame(window_layer);

	tile_layer = layer_create(frame);
	layer_set_update_proc(tile_layer, tile_layer_update_callback);
	layer_add_child(window_layer, tile_layer);

	init_game();
}
static void window_unload(Window *window) {
	layer_destroy(tile_layer);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	if(gamemode == SLOW)
		random_tile();
	else
		init_game();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
	gamemode = FAST;
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	gamemode = SLOW;
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});
  	window_set_click_config_provider(window, click_config_provider);
	window_stack_push(window, true /* Animated */);
	window_set_background_color(window, GColorBlack);

	accel_data_service_subscribe(1, accel_handler);
	accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
}

static void deinit(void) {
	accel_data_service_unsubscribe();
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
