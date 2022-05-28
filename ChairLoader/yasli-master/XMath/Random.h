#pragma once

class RandomGenerator 
{
	enum { max_value = 0x7fff };
	int value;
public:
	RandomGenerator(int val = 1) { set(val); }
	void set(int val) { value = val; }
	int get() const { return value; }
	// Generates random value [0..max_value), wasn't inline due to some bugs with optimization
	int operator()(){ return ((value = value*214013L + 2531011L) >> 16) & 0x7fff; }
	int operator()(int m) { return m ? (*this)() % m : 0; } // May by used in random_shuffle
	int operator()(int min, int max) { return min + (*this)(max - min); }
	float frnd(float x = 1.f){ return (float)((*this)()*2 - max_value)*x/(float)max_value; }
	float fabsRnd(float x = 1.f){ return (float)((*this)())*x/(float)max_value; }
	float fabsRnd(float min, float max){ return min + fabsRnd(max - min); }
	float frand(){ return (*this)()/(float)max_value; }
};

#undef random
extern RandomGenerator xm_random_generator;
inline float frnd(float x){ return xm_random_generator.frnd(x); }
inline float fabsRnd(float x){ return xm_random_generator.fabsRnd(x); }

inline unsigned rnd(unsigned m){ return xm_random_generator(m); }
