#include "tree-perf.h"
#include "box2d/box2d.h"

struct QueryChecker {
	b2DynamicTree* tree;
	int idx1;
	bool QueryCallback(int32 proxyId){
		int idx2 = (intptr_t)tree->GetUserData(proxyId);
		if(idx1 < idx2) check_pair(idx1, idx2);
		return true;
	}
};


extern "C" {
	
void time_trial_box2d(void){
	load_data();
	
	b2DynamicTree tree{};
	int32 PROXY[SIZE*SIZE];
	
	for(int i = 0; i < COUNT; i++){
		Vec2 p = POSITION[i];
		b2AABB bb = {.lowerBound = {p.x - 1, p.y - 1}, .upperBound = {p.x + 1, p.y + 1}};
		PROXY[i] = tree.CreateProxy(bb, (void*)(intptr_t)i);
	}
	
	QueryChecker checker{.tree = &tree};
	
	float sum = 0;
	for(int step = 0; step < STEPS; step++){
		update_data();
		
		step_begin();
		for(int i = 0; i < DYNAMIC_COUNT; i++){
			Vec2 p = POSITION[i], v = VELOCITY[i];
			b2AABB bb = {.lowerBound = {p.x - 1, p.y - 1}, .upperBound = {p.x + 1, p.y + 1}};
			tree.MoveProxy(PROXY[i], bb, b2Vec2(v.x*DT, v.y*DT));
		}
		
		for(int i = 0; i < DYNAMIC_COUNT; i++){
			checker.idx1 = i;
			tree.Query(&checker, tree.GetFatAABB(PROXY[i]));
		}
		step_finish(step);
	}
}

}
