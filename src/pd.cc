#include "pd.h"

#include <stdio.h>

void FIFO::init(uint32_t sz, uint32_t t_bits) {
    size = sz;
    tag_bits = t_bits;
    mask = 0;
    for (uint32_t i = 0; i < tag_bits; ++i) {
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
    for (uint32_t i = 0; i < size; ++i) {
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
    if (head >= size)
        head = 0;
    tag[head] = mask_tag;

}

PD::PD(BLOCK** blk, uint32_t prof_siz, uint32_t reuse_cnt_wid) {
    if (blk == NULL) {
        printf("error: block provided to PD is NULL\n");
        assert(0);
    }
    block = blk;
    prof_size = prof_siz;
    reuse_cnt_width = reuse_cnt_wid;

    total_set = LLC_SET;
    total_way = LLC_WAY;
    prof_set = total_set / 64;
#ifdef PD_DEBUG_FLAG
    stage_size = 512 * 1000;
#else 
    stage_size = 512 * 1024;
#endif
    visit_cnt = 0;
    prof_cnt = 0;
    max_dis = 256;
    prt_dis = max_dis;

    prof_step = max_dis / prof_size;
    prof_step_cnt = new uint32_t[prof_set];
    memset(prof_step_cnt, 0, sizeof(uint32_t) * prof_set);
    prof_tag_bits = 16;
    prof_tag = new FIFO[prof_set];
    for (uint32_t i = 0; i < prof_set; ++i) {
        prof_tag[i].init(prof_size, prof_tag_bits);
    }

    reuse_dis_cnt = new uint32_t[max_dis / reuse_cnt_width];
    memset(reuse_dis_cnt, 0, sizeof(uint32_t) * (max_dis / reuse_cnt_width));

    remain_prt_dis = new uint32_t* [total_set];
    used_flag = new bool* [total_set];
    for (uint32_t i = 0; i < total_set; ++i) {
        remain_prt_dis[i] = new uint32_t[total_way];
        memset(remain_prt_dis[i], 0, sizeof(uint32_t) * total_way);
        used_flag[i] = new bool[total_way];
        memset(used_flag[i], 0, sizeof(bool) * total_way);
    }

    printf("PD initialized!\n"); 
}

PD::~PD() { 
    delete [] prof_step_cnt; 
    delete [] prof_tag;
    delete [] reuse_dis_cnt;
    for (uint32_t i = 0; i < total_set; ++i) {
        delete [] remain_prt_dis[i];
        delete [] used_flag[i];
    }
    delete [] remain_prt_dis;
    delete [] used_flag;
    printf("PD quit!\n"); 
}

void PD::update(uint32_t set, uint32_t way) {
    // every stage_size visits, flush the prt_dis and remain_prt_dis counters
    ++visit_cnt;
    if (visit_cnt >= stage_size) {
        update_protection_distance();
        visit_cnt = 0;
    }
    // update the remain reuse distance
    for (uint32_t i = 0; i < total_way; ++i) {
        if (i == way) {
            remain_prt_dis[set][i] = prt_dis;
            used_flag[set][i] = true;
        }
        else if (remain_prt_dis[set][i] > 0) {
            --remain_prt_dis[set][i];
        }
    }

    if (set >= prof_set) return;
    ++prof_cnt;
    uint32_t reuse_dis = prof_tag[set].query(block[set][way].tag);
    if (reuse_dis < prof_size) {
        reuse_dis = reuse_dis * prof_step + prof_step_cnt[set];
        update_reuse_dis_counter(reuse_dis);
    }
    ++prof_step_cnt[set];
    if (prof_step_cnt[set] >= prof_step) {
        prof_tag[set].insert(block[set][way].tag);
        prof_step_cnt[set] = 0;
    }
}

uint32_t PD::victim(uint32_t cpu, uint64_t instr_id, uint32_t set, 
    const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type) {
    
    uint32_t way = 0;

    // fill invalid line first
    for (way = 0; way < total_way; ++way) {
        if (block[set][way].valid == false)
            break;
    }

    if (way == total_way) {
        for (way = 0; way < total_way; ++way) {
            if (remain_prt_dis[set][way] == 0) break;
        }
    }

    if (way == total_way) {
        uint32_t used_min_rpd = max_dis,   unused_min_rpd = max_dis;
        uint32_t used_min_way = total_way, unused_min_way = total_way;
        for (way = 0; way < total_way; ++way) {
            if (used_flag[set][way]) {
                if (used_min_rpd > remain_prt_dis[set][way]) {
                    used_min_rpd = remain_prt_dis[set][way];
                    used_min_way = way;
                }
            }
            else {
                if (unused_min_rpd > remain_prt_dis[set][way]) {
                    unused_min_rpd = remain_prt_dis[set][way];
                    unused_min_way = way;
                }
            }
        }
        if (unused_min_way < total_way)
            way = unused_min_way;
        else 
            way = used_min_way;
    }

    if (way == total_way) {
        cerr << "[LLC] " << __func__ << " no victim! set: " << set << endl;
        assert(0);
    }

    remain_prt_dis[set][way] = prt_dis;
    used_flag[set][way] = false;
    return way;
    
}


void PD::update_protection_distance() {
    PD_DEBUG("begin update pd");
    uint32_t old_pd = prt_dis;
    uint32_t W = total_way;
    uint32_t max_offset = max_dis / reuse_cnt_width;
    uint32_t Nt = 0;
    // Nt += reuse_dis_cnt[i];
    Nt = prof_cnt;
    prof_cnt = 0;
    PD_DEBUG("Nt: %d", Nt);
    PD_DEBUG("reuse_dis_cnt ");
    for (uint32_t i = 0; i < max_offset; ++i) {
        PD_DEBUG("%d-%d: %d, average %.1f", 
            i * reuse_cnt_width, (i+1) * reuse_cnt_width,
            reuse_dis_cnt[i], reuse_dis_cnt[i] / (float)reuse_cnt_width);
    }
    float optimal_E = 0.0;
    uint32_t optimal_off = 0;
    uint32_t Nii_acc = 0, Ni_acc = 0;
    for (uint32_t i = 0; i < max_dis; ++i) {
        uint32_t step_reuse_dis = reuse_dis_cnt[i / reuse_cnt_width] / reuse_cnt_width;
        Nii_acc += step_reuse_dis * (i + 1);
        Ni_acc += step_reuse_dis;
        float E = (float)Ni_acc / 
            (Nii_acc + (Nt - Ni_acc) * (i + 1 + W));
        if (E > optimal_E) {
            optimal_off = i;
            optimal_E = E;
        }
        PD_DEBUG("iter %d: current E %.5f, optimal_E %.5f(from iter %d)",
            i, E, optimal_E, optimal_off);
    }
    // prt_dis = (optimal_off + 1) * reuse_cnt_width;
    prt_dis = optimal_off + 1;
    PD_LOG("pd from %d to %d", old_pd, prt_dis);

    for (uint32_t i = 0; i < total_set; ++i) {
        for (uint32_t j = 0; j < total_way; ++j) {
            remain_prt_dis[i][j] = (prt_dis < remain_prt_dis[i][j]) 
                                 ?  prt_dis : remain_prt_dis[i][j];
        }
    }

    memset(reuse_dis_cnt, 0, sizeof(uint32_t) * (max_offset));
    PD_DEBUG("end update pd");
}

void PD::update_reuse_dis_counter(uint32_t reuse_dis) {
    if (reuse_dis >= max_dis) return;
    uint32_t reuse_cnt_offset = reuse_dis / reuse_cnt_width;
    ++reuse_dis_cnt[reuse_cnt_offset];
}

