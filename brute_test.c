#include "tree-perf.h"

void time_trial_brute(void){
	load_data();
	
	for(int step = 0; step < STEPS; step++){
		update_data();
		
		step_begin();
		for(int i = 0; i < DYNAMIC_COUNT; i++){
			for(int j = i + 1; j < COUNT; j++) check_pair(i, j);
		}
		step_finish(step);
	}
}
