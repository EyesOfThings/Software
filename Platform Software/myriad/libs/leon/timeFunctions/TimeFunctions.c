/*!
    \file
 
    \brief     Library for common time functions. File
*/

#include <TimeFunctions.h>

struct tm _year88 = {.tm_hour = 0, .tm_min = 0, .tm_sec = 0, .tm_year = 88, .tm_mon = 0, .tm_mday = 1};
struct tm _initialCurrentTime = {.tm_hour = 0, .tm_min = 0, .tm_sec = 0, .tm_year = 88, .tm_mon = 0, .tm_mday = 1};
double _initialCurrentTimeSeconds=0;

/*!
 *  \brief Print current time
*/

void printCurrentTime(){
    struct tm currentTime=getCurrentTime();
    printf( "%s", asctime(&currentTime));

}

/*!
 *  \brief Get current time in struct tm parameter
 *  \return         current time in struct tm format
*/
struct tm getCurrentTime(){
    struct tm currentTime={.tm_hour = _initialCurrentTime.tm_hour, .tm_min = _initialCurrentTime.tm_min,
        .tm_sec = _initialCurrentTime.tm_sec, .tm_year = _initialCurrentTime.tm_year, .tm_mon = _initialCurrentTime.tm_mon, .tm_mday = _initialCurrentTime.tm_mday};
    
    time_t timer;
    double seconds;
    time(&timer); /* get current time; same as: timer = time(NULL)  */
    seconds = difftime(timer, mktime(&_year88));
    
    currentTime.tm_sec=currentTime.tm_sec+(seconds-_initialCurrentTimeSeconds);
    
    mktime(&currentTime);
    
    return currentTime;
}


/*!
 *  \brief Get current time in time_t value (seconds since 01-jan-1988 00h00m00s)
 *  \return         current time in time_t format
*/
double getCurrentTime_t(){
    struct tm currentTime=getCurrentTime();
    double result1=mktime(&currentTime);
    printf("result1: %.f \n",result1);
    //time_t result=mktime(&currentTime);
    //printf("result: %.f \n",result);
    
    return result1;
}


/*!
 *  \brief Get current time in char*
 *  \return         current time in char*
*/
char* getCurrentTime_char(){
    struct tm currentTime=getCurrentTime();
    return asctime(&currentTime);

}


/*!
 *  \brief Set current time of the application
 *  \param[in]      hour of current time (0-23)
 *  \param[in]      min of current time (0-59)
 *  \param[in]      sec of current time (0-59)
 *  \param[in]      year of current time (1900-inf)
 *  \param[in]      month of current time (1-12)
 *  \param[in]      day of current time (1-31 depending on month)
*/
void setCurrentTime(int hour, int min, int sec, int year, int mon, int mday){
    //change initial current time
    _initialCurrentTime.tm_hour=hour;
    _initialCurrentTime.tm_min=min;
    _initialCurrentTime.tm_sec=sec;
    _initialCurrentTime.tm_year=year-1900;
    _initialCurrentTime.tm_mon=mon-1;
    _initialCurrentTime.tm_mday=mday;
    
    mktime(&_initialCurrentTime);
    
    //change number of seconds since start
    time_t timer;
    time(&timer); /* get current time; same as: timer = time(NULL)  */
    _initialCurrentTimeSeconds = difftime(timer, mktime(&_year88));
    
}


