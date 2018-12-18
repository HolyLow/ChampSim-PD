#ifndef PD_H_
 #define PD_H_

#include "cache.h"

typedef struct FIFO {
    FIFO() {}
    ~FIFO() {}

    uint32_t size;                  // size of the fifo
    uint32_t tag_bits;              // the used bit length of the tags

}FIFO;

class PD {
public:
    PD(uint32_t prof_siz = 32, uint32_t reuse_cnt_wid = 4);
    ~PD();
    void update(uint32_t set, uint32_t way);
    uint32_t victim(uint32_t cpu, uint64_t instr_id, uint32_t set, const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type);

private:

    uint32_t set;                   // llc cache set number
    uint32_t prof_set;              // profiled set number
    
    uint32_t max_dis;               // max profiled distance
    uint32_t prt_dis;               // protection distance
    
    // uint32_t prof_step;             // step of profiling. only one visit among #prof_step visits will be inserted to the prof_tag
    // uint32_t prof_step_cnt;         // profiling cnt among a profiling step

    uint32_t *reuse_dis_cnt;        // reuse distance cnt (aka the reuse distance distribution)
    uint32_t reuse_cnt_width;       // sets within continuous #reuse_cnt_width will be cnted by the same reuse_dis_cnter

    uint32_t prof_size;             // the size of the prof_tag fifo array
    FIFO **prof_tag;                // the profiled tags, stored in fifo structure
    uint32_t **remain_prt_dis;      // remained protection distance for all the sets

    uint64_t stage;                 // the protection distance will be recomputed every #stage iterations
    uint64_t stage_cnt;             // the stage cnter

};


#endif // PD_H_
