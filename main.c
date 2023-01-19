#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SOKOL_IMPL
#include "sokol_time.h"

#include "chipmunk/chipmunk.h"

#define MAX_SIZE 256
#define DYNAMIC_COUNT (COUNT/1)
#define STEPS 200
#define TIMING_SIZE (STEPS/2)

static int SIZE = 256;

static float DT = 1.0f/60.0f;
static float V_FACTOR = 10.0f/100.0f;
static float SPACING = 1.80f;
static float RADIUS = 1.00f;

typedef struct {float x, y;} Vec2;
static Vec2 POSITION[MAX_SIZE*MAX_SIZE];
static Vec2 VELOCITY[MAX_SIZE*MAX_SIZE];
static int COUNT;
static uint32_t PASS_COUNT, FAIL_COUNT;
static uint16_t COL_HASH;
static float TIMING[TIMING_SIZE];

static void load_data(void){
	// Load 256x256 blue noise distribution image from: http://nothings.org/gamedev/blue_noise/
	int w, h, comp;
	uint8_t* pixels = stbi_load("blue_noise_placement_256_0.png", &w, &h, &comp, 0);
	assert(pixels);
	
	COUNT = COL_HASH = 0;
	for(int i = 0; i < SIZE*SIZE; i++) COUNT += (pixels[4*i + 3] > 0);
	
	int cursor = 0;
	for(int i = 0; i < SIZE*SIZE; i++){
		int j = (i*47 ^ 389) % (SIZE*SIZE);
		int x = j % SIZE, y = j / SIZE;
		int idx = 4*(w*y + x);
		if(pixels[idx + 3] == 0) continue;
		
		Vec2 pos = {
			SPACING*(x + pixels[idx + 0]/255.0f - SIZE/2),
			SPACING*(y + pixels[idx + 1]/255.0f - SIZE/2),
		};
		
		POSITION[cursor] = pos;
		VELOCITY[cursor] = (Vec2){0, 0};
		cursor++;
	}
	
	stbi_image_free(pixels);
}

static void update_data(void){
	for(int i = 0; i < DYNAMIC_COUNT; i++){
		VELOCITY[i].x = +POSITION[i].y*V_FACTOR;
		VELOCITY[i].y = -POSITION[i].x*V_FACTOR;
		POSITION[i].x += VELOCITY[i].x*DT;
		POSITION[i].y += VELOCITY[i].y*DT;
	}
	
	PASS_COUNT = 0;
	FAIL_COUNT = 0;
}

static inline void check_pair(int i, int j){
	Vec2 a = POSITION[i], b = POSITION[j];
	float dx = a.x - b.x, dy = a.y - b.y;
	if(dx*dx + dy*dy < 4*RADIUS*RADIUS){
		COL_HASH += i + j;
		PASS_COUNT++;
	} else {
		FAIL_COUNT++;
	}
}

// static void print_results(int step, double timing){
// 	printf("% 2d, % 10.5f, 0x%04X, %d, %d, % 5.2f%%\n",
// 	step, 1e3*timing, COL_HASH, PASS_COUNT, FAIL_COUNT, 100.0f*PASS_COUNT/(float)(PASS_COUNT + FAIL_COUNT + FLT_MIN));
// }

static void time_trial_brute(void){
	load_data();
	
	for(int step = 0; step < STEPS; step++){
		update_data();
		
		uint64_t start_step = stm_now();
		for(int i = 0; i < DYNAMIC_COUNT; i++){
			for(int j = i + 1; j < COUNT; j++) check_pair(i, j);
		}
		TIMING[step % TIMING_SIZE] = stm_sec(stm_since(start_step));
	}
}

cpBB bb_func(void *obj){
	int i = (int)(intptr_t)obj - 1;
	return cpBBNewForCircle(cpv(POSITION[i].x, POSITION[i].y), RADIUS);
}

cpVect velocity_func(void *obj){
	int i = (int)(intptr_t)obj - 1;
	return cpv(VELOCITY[i].x, VELOCITY[i].y);
}

static cpCollisionID query_func(void *obj1, void *obj2, cpCollisionID id, void *data){
	check_pair((intptr_t)obj1 - 1, (intptr_t)obj2 - 1);
	return 0;
}

static void time_trial_chipmunk(void){
	load_data();
	
	cpSpatialIndex* static_tree = cpBBTreeNew(bb_func, NULL);
	cpSpatialIndex* dynamic_tree = cpBBTreeNew(bb_func, 1 ? static_tree : NULL);
	cpBBTreeSetVelocityFunc(dynamic_tree, velocity_func);
	
	for(int i = 0; i < COUNT; i++){
		cpSpatialIndex* tree = (i < DYNAMIC_COUNT ? dynamic_tree : static_tree);
		cpSpatialIndexInsert(tree, (void*)(intptr_t)(i + 1), i);
	}
	
	for(int step = 0; step < STEPS; step++){
		update_data();
		
		uint64_t start_step = stm_now();
		cpSpatialIndexReindexQuery(dynamic_tree, query_func, NULL);
		TIMING[step % TIMING_SIZE] = stm_sec(stm_since(start_step));
	}
}

int main(int argc, const char* argv[]){
	stm_setup();
	
	for(float size = 8; size < MAX_SIZE + 1; size *= pow(2, 0.5)){
		SIZE = round(size);
		time_trial_chipmunk();
		
		float sum = 0;
		for(int i = 0; i < TIMING_SIZE; i++) sum += TIMING[i];
		printf("% 6d, % 6e, 0x%04X\n", COUNT, 1e3*sum/TIMING_SIZE, COL_HASH);
	}
	
	return EXIT_SUCCESS;
}
