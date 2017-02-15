/*
 * File:   timeFunctions.h
 * Author: espiaran
 *
 * Created on 4 de noviembre de 2015, 13:53
 */

#ifndef TIMEFUNCTIONS_H
#define	TIMEFUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

//Functions definitions
void printCurrentTime();
struct tm getCurrentTime();
double getCurrentTime_t();
char* getCurrentTime_char();
void setCurrentTime(int hour, int min, int sec, int year, int mon, int mday);

#ifdef __cplusplus
}
#endif

#endif	/* TIMEFUNCTIONS_H */
