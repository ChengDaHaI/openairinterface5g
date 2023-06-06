/*! \file o1_csv.c
 * \brief metrics saving in csv file
 * \author  Hai Cheng
 * \date 2023, June 2
 * \email: cheng.hai@northeastern.edu
 * \version 0.1
 * \company NEU
 * @ingroup WIOT
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// for timestamp
#include <time.h>
#include <sys/timeb.h>

#include "openair2/LAYER2/NR_MAC_gNB/mac_proto.h"

extern RAN_CONTEXT_t RC;

/* msleep(): Sleep for the requested number of milliseconds. */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

long int get_time_milliseconds(void) {

    struct timeb tmb;
    ftime(&tmb);

    return tmb.time * 1000.0 + ((long int) tmb.millitm);
}

void dump_ue_mac_stats_to_csv(gNB_MAC_INST *gNB, char *ue_metrics_csv_filepath, bool reset_rsrp)
{
  int num = 1;

  char csv_header[] = "Timestamp, Frame, Slot, IMSI, RNTI, UID, UE_No, DL_THR, UL_THR, ,PH_dB, PCMAX_dBm, AVG_RSRP, CQI, RI, raw_rssi, ul_rssi, dl_max_mcs, sched_ul_bytes, estimated_ul_buffer, num_total_bytes, dl_pdus_total, ,BLER, MCS, dlsch_errors, dlsch_total_bytes, dlsch_current_bytes, dlsch_total_rbs, dlsch_current_rbs, dl_num_mac_sdu, ,BLER, MCS, ulsch_errors, ulsch_total_bytes, ulsch_current_bytes, ulsch_total_rbs, ulsch_current_rbs, ul_num_mac_sdu, ulsch_total_bytes_scheduled, ,\n";

  pthread_mutex_lock(&gNB->UE_info.mutex);
  long timestamp = get_time_milliseconds();

  UE_iterator(gNB->UE_info.list, UE) {
    NR_UE_sched_ctrl_t *sched_ctrl = &UE->UE_sched_ctrl;
    NR_mac_stats_t *stats = &UE->mac_stats;
    const int avg_rsrp = stats->num_rsrp_meas > 0 ? stats->cumul_rsrp / stats->num_rsrp_meas : 0;

    char metrics[1000] = {0};

    // char ue_id_str[1];
    // char ue_metrics_csv_filepath[100] = {0};
    // strcat(ue_metrics_csv_filepath, snprintf(ue_id_str, 1, "%d", UE->uid));
    // strcat(ue_metrics_csv_filepath, '_metrics.csv');
    // char ue_metrics_csv_filepath[] = "/home/haicheng/run_scripts/ue_metrics.csv";
    char ue_imsi[] = "0010100000107XX";

    snprintf(metrics,
            1000,
            "%ld,  %d,  %d,  %s,  %04x,  %d, %d,  %f, %f, , %d, %d,  %d,  %d,  %d,  %d,  %u,  %u,  %d,  %d,  %"PRIu32",  %"PRIu16", ,%.5f, %d, %"PRIu64", %"PRIu64", %"PRIu32", %"PRIu32",  %"PRIu32", %"PRIu32", ,%.5f, %d, %"PRIu64", %"PRIu64", %"PRIu32", %"PRIu32", %"PRIu32", %"PRIu32", %"PRIu64", , \n",
            timestamp,
            gNB->frame,
            gNB->slot,
            ue_imsi,
            UE->rnti,
            UE->uid,
            num++,
            UE->dl_thr_ue,
            UE->ul_thr_ue,
            // UE sched ctrl
            sched_ctrl->ph,
            sched_ctrl->pcmax,
            avg_rsrp,
            sched_ctrl->CSI_report.cri_ri_li_pmi_cqi_report.wb_cqi_1tb,
            sched_ctrl->CSI_report.cri_ri_li_pmi_cqi_report.ri+1,
            sched_ctrl->raw_rssi,
            sched_ctrl->ul_rssi,
            sched_ctrl->dl_max_mcs,
            sched_ctrl->sched_ul_bytes,
            sched_ctrl->estimated_ul_buffer,
            sched_ctrl->num_total_bytes,
            sched_ctrl->dl_pdus_total,
            // DL stats
            sched_ctrl->dl_bler_stats.bler,
            sched_ctrl->dl_bler_stats.mcs,
            stats->dl.errors,
            stats->dl.total_bytes,
            stats->dl.current_bytes,
            stats->dl.total_rbs,
            stats->dl.current_rbs,
            stats->dl.num_mac_sdu,
            // UL stats
            sched_ctrl->ul_bler_stats.bler,
            sched_ctrl->ul_bler_stats.mcs,
            stats->ul.errors,
            stats->ul.total_bytes,
            stats->ul.current_bytes,
            stats->ul.total_rbs,
            stats->ul.current_rbs,
            stats->ul.num_mac_sdu,
            stats->ulsch_total_bytes_scheduled
            );

    if (reset_rsrp) {
      stats->num_rsrp_meas = 0;
      stats->cumul_rsrp = 0;
    }
    // // FILE *csv_file;
    FILE *csv_file;
    if ( fopen(ue_metrics_csv_filepath, "r")) {
      // file exist, write metrics
        csv_file = fopen(ue_metrics_csv_filepath, "a");
        fprintf(csv_file, metrics);
    } else {
      // file not exist, create file and write header
      printf("file not exists: write header\n");
      csv_file = fopen(ue_metrics_csv_filepath, "w");
      fprintf(csv_file, csv_header);
    }
    fclose(csv_file);
  }
  pthread_mutex_unlock(&gNB->UE_info.mutex);
}

