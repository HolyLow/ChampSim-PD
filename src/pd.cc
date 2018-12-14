#include "pd.h"

#include <stdio.h>


PD::PD(uint32_t sets, uint32_t prof_ratio, uint32_t prof_siz, uint32_t reuse_cnt_stp, uint32_t stge) {
    printf("PD initialized!\n"); 
}

PD::~PD() { 
    printf("PD quit!\n"); 
}

void PD::update(uint32_t set, uint32_t way) {

}

uint32_t PD::victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type) {
    
}

