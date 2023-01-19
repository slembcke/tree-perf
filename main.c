#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define SOKOL_IMPL
#include "sokol_time.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "tree-perf.h"

#include "chipmunk/chipmunk.h"

int SIZE = 256;

float DT = 1.0f/60.0f;
float V_FACTOR = 10.0f/100.0f;
float SPACING = 1.80f;
float RADIUS = 1.00f;

Vec2 POSITION[MAX_SIZE*MAX_SIZE];
Vec2 VELOCITY[MAX_SIZE*MAX_SIZE];
int COUNT;
unsigned PASS_COUNT, FAIL_COUNT;
unsigned COL_HASH;
float TIMING[TIMING_SIZE];

void load_data(void){
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

void update_data(void){
	for(int i = 0; i < DYNAMIC_COUNT; i++){
		VELOCITY[i].x = +POSITION[i].y*V_FACTOR;
		VELOCITY[i].y = -POSITION[i].x*V_FACTOR;
		POSITION[i].x += VELOCITY[i].x*DT;
		POSITION[i].y += VELOCITY[i].y*DT;
	}
	
	PASS_COUNT = 0;
	FAIL_COUNT = 0;
}

static uint64_t STEP_START;
void step_begin(void){
	STEP_START = stm_now();
}

void step_finish(int step){
	TIMING[step % TIMING_SIZE] = stm_sec(stm_since(STEP_START));
	printf("%d: %d/%d\r", COUNT, step, STEPS);
	fflush(stdout);
}

extern void time_trial_brute(void);
extern void time_trial_chipmunk(void);
extern void time_trial_box2d(void);

static void run_time_trial(const char* label, void (*trial_func)(void)){
	printf("%s:\n", label);
	
	for(float size = 8; size < MAX_SIZE + 1; size *= pow(2, 0.5)){
		SIZE = round(size);
		trial_func();
		
		float sum = 0;
		for(int i = 0; i < TIMING_SIZE; i++) sum += TIMING[i];
		printf("% 6d, % 6e, 0x%04X\n", COUNT, 1e3*sum/TIMING_SIZE, COL_HASH & 0xFFFF);
	}
}

int main(int argc, const char* argv[]){
	stm_setup();
	
	run_time_trial("CHIPMUNK", time_trial_chipmunk);
	run_time_trial("BOX2D", time_trial_box2d);
	run_time_trial("BRUTE", time_trial_brute);
	
	return EXIT_SUCCESS;
}
