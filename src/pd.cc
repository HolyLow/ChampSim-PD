#include "pd.h"

#include <stdio.h>

void FIFO::init(uint32_t sz, uint32_t t_bits) {
    size = sz;
    tag_bits = t_bits;
    mask = 0;
    for (int i = 0; i < tag_bits; ++i) {
        mask <<= 1;
        mask |= 1;
    }
    tag = new uint64_t[size];
    memset(tag, 0, sizeof(uint64_t) * size);
    head = 0;
}

FIFO::~FIFO() {
    if (tag != NULL)
        delete [] tag;
}

uint32_t FIFO::query(uint64_t t) {
    uint32_t current;
    uint64_t mask_tag = mask & t;
    for (int i = 0; i < size; ++i) {
        if (head >= i)
            current = head - i;
        else
            current = size + head - i;
        if (mask_tag == tag[current])
            return i;
    }
    return size;
}

void FIFO::insert(uint64_t t) {
    uint64_t mask_tag = mask & t;
    ++head;
    if (head == size)
        head = 0;
    tag[head] = mask_tag;

}

PD::PD(BLOCK** blk, uint32_t prof_siz, uint32_t reuse_cnt_wid) {
    if (blk == NULL) {
        printf("error: block provided to PD is NULL\n");
        exit(1);
    }
    block = blk;
    prof_size = prof_siz;
    reuse_cnt_width = reuse_cnt_wid;

    total_set = LLC_SET;
    total_way = LLC_WAY;
    prof_set = sets / 64;
    stage_size = 512 * 1024;
    visit_cnt = 0;
    max_dis = 256;
    prt_dis = max_dis;

    prof_step = max_dis / prof_siz;
    prof_step_cnt = new uint32_t[prof_set];
    memset(prof_step_cnt, 0, sizeof(uint32_t) * prof_set);
    prof_tag_bits = 16;
    prof_tag = new FIFO[10];
    for (int i = 0; i < prof_set; ++i) {
        prof_tag[i].init(prof_size, prof_tag_bits);
    }

    reuse_dis_cnt = new uint32_t[max_dis / reuse_cnt_width];

    remain_prt_dis = new uint32_t* [prof_set];
    used_flag = new bool* [prof_set];
    for (int i = 0; i < prof_set; ++i) {
        remain_prt_dis[i] = new uint32_t[total_way];
        memset(remain_prt_dis[i], 0, sizeof(uint32_t) * total_way);
        used_flag[i] = new bool[total_way];
        memset(used_flag[i], 0, sizeof(bool) * total_way);
    }

    printf("PD initialized!\n"); 
}

PD::~PD() { 
    printf("PD quit!\n"); 
}

void PD::update(uint32_t set, uint32_t way) {
    // flush the reuse distance and the counters if reaches the stage_size
    ++visit_cnt;
    if (visit_cnt >= stage_size) {
        visit_cnt = 0;
        flush();
    }
    // update the remain reuse distance
    for (int i = 0; i < total_way; ++i) {
        if (i == way) {
            remain_prt_dis[set][i] = prt_dis;
            used_flag[set][i] = true;
        }
        else if (remain_prt_dis[set][i] != 0) {
            --remain_prt_dis[set][i];
        }
    }

    if (set >= prof_set) continue;
    uint32_t reuse_dis = prof_tag[set].query(block[set][way].tag);
    if (reuse_dis >= prof_size) continue;
    reuse_dis = reuse_dis * prof_step + prof_step_cnt;
    update_reuse_dis_counter(reuse_dis);
    ++prof_step_cnt[set];
    if (prof_step_cnt[set] == prof_step) {
        prof_tag[set].insert(block[set][way].tag);
        prof_step_cnt[set] = 0;
    }
}

uint32_t PD::victim(uint32_t cpu, uint64_t instr_id, uint32_t set, 
    const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type) {
    
    
}

void PD::flush() {

}

void PD::update_reuse_distance() {

}

void PD::update_reuse_dis_counter(uint32_t reuse_dis) {
    if (reuse_dis >= max_dis) return;
    uint32_t reuse_cnt_offset = reuse_dis / reuse_cnt_width;
    ++reuse_dis_cnt[reuse_cnt_offset];
}

