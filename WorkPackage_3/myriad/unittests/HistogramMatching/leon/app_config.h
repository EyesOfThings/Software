/**
 * @brief    Application configuration Leon header
 */
#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/**
 * Setup all clock configurations needed by the application and also the DDR.
 * @return    OS_MYR_DRV_SUCCESS (i.e. 0) on success, non-zero otherwise
 */
int initClocksAndMemory(void);

#endif  // _APP_CONFIG_H_
