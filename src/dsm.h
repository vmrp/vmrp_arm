#ifndef _DSM_H
#define _DSM_H

#include "./mr/include/mrporting.h"


//------------------------------------------------

#define MT6235

/*请不要修改这些值*/
#if (defined(MT6223P) || defined(MT6223) || defined(MT6223P_S00))
#define DSM_PLAT_VERSION (2) /*手机平台区分(1~99)*/
#elif (defined(MT6226) || defined(MT6226M) || defined(MT6226D))
#define DSM_PLAT_VERSION (4) /*手机平台区分(1~99)*/
#elif (defined(MT6228))
#define DSM_PLAT_VERSION (5) /*手机平台区分(1~99)*/
#elif (defined(MT6225))
#define DSM_PLAT_VERSION (3) /*手机平台区分(1~99)*/
#elif (defined(MT6230))
#define DSM_PLAT_VERSION (6) /*手机平台区分(1~99)*/
#elif (defined(MT6227) || defined(MT6227D))
#define DSM_PLAT_VERSION (7)
#elif (defined(MT6219))
#define DSM_PLAT_VERSION (1)
#elif (defined(MT6235) || defined(MT6235B))
#define DSM_PLAT_VERSION (8)
#elif (defined(MT6229))
#define DSM_PLAT_VERSION (9)
#elif (defined(MT6253) || defined(MT6253T))
#define DSM_PLAT_VERSION (10)
#elif (defined(MT6238))
#define DSM_PLAT_VERSION (11)
#elif (defined(MT6239))
#define DSM_PLAT_VERSION (12)
#else
#error PLATFORM NOT IN LIST PLEASE CALL SKY TO ADD THE PLATFORM
#endif

#ifdef DSM_IDLE_APP
#define DSM_FAE_VERSION (180) /*由平台组统一分配版本号，有需求请联系平台组*/
#else
#define DSM_FAE_VERSION (182) /*由平台组统一分配版本号，有需求请联系平台组*/
#endif

//---------------------------------
void dsm_init(uint16 *scrBuf);

int32 mr_exit(void);

void SetDsmWorkPath(const char *path);

#endif
