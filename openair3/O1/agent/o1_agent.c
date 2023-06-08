/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include "o1_csv.h"
#include "o1_json.h"
#include "o1_agent.h"

extern RAN_CONTEXT_t RC;

o1_agent_t* o1_init_agent(char* url, uint16_t report_interval, char* saving_path, char* mode)
{
  assert(url != NULL);
  // TODO: Check it's valid url (http or https)

  printf("[O1 AGENT]: Initializing ... \n");
  o1_agent_t* ag = calloc(1, sizeof(*ag));
  assert(ag != NULL && "Memory exhausted");
  ag->url = malloc(strlen(url) * sizeof(char));
  strcpy(ag->url, url);
  ag->report_interval = report_interval;
  ag->saving_path = malloc(strlen(saving_path) * sizeof(char));
  strcpy(ag->saving_path, saving_path);
  ag->mode = malloc(strlen(mode) * sizeof(char));
  strcpy(ag->mode, mode);

  return ag;
}

void o1_free_agent(o1_agent_t* ag)
{
  ag->agent_stopped = true;
  //   ag->stop_token = true;
  //   while (ag->agent_stopped == false) {
  //     usleep(1000);
  //   }
  free(ag->url);
  free(ag);
}

void o1_start_agent(o1_agent_t* ag)
{
  if (strcmp(ag->mode, "json") == 0)
  {
    printf("O1 Mode set to json!");
    o1_send_json(ag->url, gen_pnf());

    while (!ag->agent_stopped) {
      msleep(ag->report_interval);
      o1_send_json(ag->url, gen_hb());
      for (int i = 0; i < RC.nb_nr_macrlc_inst; i++) {
        gNB_MAC_INST *gNB = RC.nrmac[i];
        struct pm_fields pmf[MAX_MOBILES_PER_GNB + 1];
        o1_copy_mac_stats_pmf(gNB, pmf);

        for (int ueIndex = 0; ueIndex < MAX_MOBILES_PER_GNB + 1; ueIndex++) {
          if (pmf[ueIndex].rnti) {
            o1_send_json(ag->url, gen_pm(pmf[ueIndex]));
          }
        }
      }
    }
  }
  else if (strcmp(ag->mode, "csv") == 0)
  {
    printf("O1 Mode set to csv!");
    while ((!ag->agent_stopped) && (oai_exit == 0)) {
      msleep(ag->report_interval);
      for (int i = 0; i < RC.nb_nr_macrlc_inst; i++) {
        gNB_MAC_INST *gNB = RC.nrmac[i];
        o1_save_csv(gNB,ag->saving_path);
      }
    }
  }
  else
  {
    printf("O1 Agent Mode Error!\n");
    exit(0);
  }
}
