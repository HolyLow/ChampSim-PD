#include "pd.h"

#include <stdio.h>


PD::PD(uint32_t prof_siz, uint32_t reuse_cnt_wid) {
    prof_size = prof_siz;
    reuse_cnt_width = reuse_cnt_wid;

    set = LLC_SET;
    prof_set = sets / 64;
    stage = 512 * 1024;
    stage_cnt = 0;
    max_dis = 256;
    prt_dis = max_dis;

    // prof_step = 

    // reuse_cnt_width = 4;
    reuse_dis_cnt = new uint32_t[max_dis / reuse_cnt_width];

    remain_prt_dis = new uint32_t* [prof_set];
    for (int i = 0; i < prof_set; ++i) {
        remain_prt_dis[i] = new uint32_t[LLC_WAY];
        memset(remain_prt_dis[i], 0, sizeof(uint32_t) * LLC_WAY);
    }




    printf("PD initialized!\n"); 
}

PD::~PD() { 
    printf("PD quit!\n"); 
}

void PD::update(uint32_t set, uint32_t way) {

}

uint32_t PD::victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type) {
    
}

