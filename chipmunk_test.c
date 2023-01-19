#include "tree-perf.h"
#include "chipmunk/chipmunk.h"

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

void time_trial_chipmunk(void){
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
		
		step_begin();
		cpSpatialIndexReindexQuery(dynamic_tree, query_func, NULL);
		step_finish(step);
	}
}
