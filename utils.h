#ifndef UTILS_H
#define UTILS_H

//renamed from "rand" because of conflict with stdlib.h. :(
void     init_rnd(uint32_t seed);
uint32_t rnd(uint32_t max);

void     delay(uint32_t us);

//float sqrt(const float m);
//double sin(double x); //input must be 0.0 to 2*pi
//double cos(double x); //input must be 0.0 to 2*pi

#endif
