/* 
 * \file o1_csv.h
 * \brief header file for metrics saving in csv file
 * \author  Hai Cheng
 * \date 2023, June 7
 * \email: cheng.hai@northeastern.edu
 * \version 0.1
 * \company NEU
 * @ingroup WIOT
 */

#ifndef O1_CSV_h
#define O1_CSV_h

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openair2/LAYER2/NR_MAC_gNB/mac_proto.h"



long get_time_milliseconds();
int msleep(long msec);

size_t o1_copy_mac_stats_csv(gNB_MAC_INST *gNB, char *output, size_t strlen, bool reset_rsrp);
void o1_save_csv(gNB_MAC_INST *gNB, char *ue_metrics_csv_filepath);

#endif