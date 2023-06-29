/*
 * \file o1_csv.c
 * \brief metrics saving in csv file
 * \author  Hai Cheng
 * \date 2023, June 2
 * \email: cheng.hai@northeastern.edu
 * \version 0.1
 * \company NEU
 * @ingroup WIOT
 */

#include "o1_json.h"

#define MAC_STATS_BUF_LEN 16000


long get_time_milliseconds() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long ms = ts.tv_sec * 1000 + round(ts.tv_nsec / 1.0e6);
    return ms;
}

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

size_t o1_copy_mac_stats_csv(gNB_MAC_INST *gNB, char *output, size_t strlen, bool reset_rsrp)
{
  int num = 1;
  const char *begin = output;
  const char *end = output + strlen;

  pthread_mutex_lock(&gNB->UE_info.mutex);

  UE_iterator(gNB->UE_info.list, UE) {
    NR_UE_sched_ctrl_t *sched_ctrl = &UE->UE_sched_ctrl;
    NR_mac_stats_t *stats = &UE->mac_stats;
    const int avg_rsrp = stats->num_rsrp_meas > 0 ? stats->cumul_rsrp / stats->num_rsrp_meas : 0;

    char ue_imsi[] = "0010100000107XX";
    long timestamp = get_time_milliseconds();
    output += snprintf(output,
            end - output,
            "%ld,  %d,  %d,  %s,  %04x,  %d, %d,  %f, %f, , %d, %d,  %d,  %d,  %d,  %d,  %u,  %u,  %d,  %d,  %"PRIu32",  %"PRIu16", ,%.5f, %d, %"PRIu64", %"PRIu64", %"PRIu32", %"PRIu32", %"PRIu32", %"PRIu32", %"PRIu64", %"PRIu64", ,%.5f, %d, %"PRIu64", %"PRIu64", %"PRIu32", %"PRIu32", %"PRIu32", %"PRIu32", , \n",
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
            stats->dl.lc_bytes[4],
            stats->dl.lc_bytes[5],
            // UL stats
            sched_ctrl->ul_bler_stats.bler,
            sched_ctrl->ul_bler_stats.mcs,
            stats->ul.errors,
            stats->ul.total_bytes,
            stats->ul.current_bytes,
            stats->ul.total_rbs,
            stats->ul.current_rbs,
            stats->ul.num_mac_sdu
            );

    if (reset_rsrp) {
      stats->num_rsrp_meas = 0;
      stats->cumul_rsrp = 0;
    }
  }
  pthread_mutex_unlock(&gNB->UE_info.mutex);
  return output - begin;
}

void o1_save_csv(gNB_MAC_INST *gNB, char *ue_metrics_csv_filepath)
{

  char csv_header[] = "Timestamp, Frame, Slot, IMSI, RNTI, UID, UE_No, DL_THR, UL_THR, ,PH_dB, PCMAX_dBm, AVG_RSRP, CQI, RI, raw_rssi, ul_rssi, dl_max_mcs, sched_ul_bytes, estimated_ul_buffer, num_total_bytes, dl_pdus_total, ,dl_BLER, dl_MCS, dlsch_errors, dlsch_total_bytes, dlsch_current_bytes, dlsch_total_rbs, dlsch_current_rbs, dl_num_mac_sdu, dl_lc_bytes_4, dl_lc_bytes_5, ,ul_BLER, ul_MCS, ulsch_errors, ulsch_total_bytes, ulsch_current_bytes, ulsch_total_rbs, ulsch_current_rbs, ul_num_mac_sdu, ulsch_total_bytes_scheduled, ul_lc_bytes_4, ul_lc_bytes_5, ,\n";

  int size = 0;
  char output[MAC_STATS_BUF_LEN] = {0};
  const char *end = output + MAC_STATS_BUF_LEN;

  FILE *file = fopen(ue_metrics_csv_filepath,"a+");

  AssertFatal(file!=NULL,"Cannot open %s, error %s\n",ue_metrics_csv_filepath, strerror(errno));

  if (NULL !=file){
    fseek(file,0,SEEK_END);
    size = ftell(file);
  }
  if (size == 0){
    fprintf(file,csv_header);
  }
  // while (oai_exit == 0) {
    char *p = output;
    p += o1_copy_mac_stats_csv(gNB, p, end - p, true);
    fwrite(output, p - output, 1, file);
    fflush(file);
    // msleep(report_interval);
    fseek(file,0,SEEK_SET);
  // }
  fclose(file);
}
