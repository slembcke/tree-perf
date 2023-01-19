#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SIZE 256
#define DYNAMIC_COUNT (COUNT/1)
#define STEPS 200
#define TIMING_SIZE (STEPS/2)

extern int SIZE;

extern float DT;
extern float V_FACTOR;
extern float SPACING;
extern float RADIUS;

typedef struct {float x, y;} Vec2;
extern Vec2 POSITION[MAX_SIZE*MAX_SIZE];
extern Vec2 VELOCITY[MAX_SIZE*MAX_SIZE];
extern int COUNT;
extern unsigned PASS_COUNT, FAIL_COUNT;
extern unsigned COL_HASH;
extern float TIMING[TIMING_SIZE];

extern void load_data(void);
extern void update_data(void);
extern void step_begin(void);
extern void step_finish(int step);

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

#ifdef __cplusplus
}
#endif
