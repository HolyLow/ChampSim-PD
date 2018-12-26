#ifndef PD_H_
 #define PD_H_

#include "cache.h"

class FIFO {
public:
    FIFO() {}
    void init(uint32_t sz, uint32_t t_bits);
    
    ~FIFO();
    uint32_t query(uint64_t t);
    void insert(uint64_t t);

private:
    uint32_t size;                  // size of the fifo
    uint32_t tag_bits;              // the used bit length of the tags
    uint64_t mask;                  // the mask for the tag
    uint64_t* tag;                  // the tag array
    uint32_t head;

};

class PD {
public:
    PD(BLOCK** blk, uint32_t prof_siz = 32, uint32_t reuse_cnt_wid = 4);
    ~PD();
    void update(uint32_t set, uint32_t way);
    uint32_t victim(uint32_t cpu, uint64_t instr_id, uint32_t set, 
        const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type);

private:
    void flush();
    void update_reuse_distance();
    void update_reuse_dis_counter(uint32_t reuse_dis);

    BLOCK ** block;

    uint32_t total_set;             // llc cache set number
    uint32_t prof_set;              // profiled set number
    uint32_t total_way;             // llc cache way number
    
    uint32_t max_dis;               // max profiled distance
    uint32_t prt_dis;               // protection distance
    uint32_t **remain_prt_dis;      // remained protection distance for all the sets
    bool **used_flag;               // flag for each ways, indicate if the way has been used
    
    uint32_t prof_step;             // step of profiling, only one visit among #prof_step visits will be inserted to the prof_tag
    uint32_t* prof_step_cnt;        // cnter of profiling step for each prof_set
    uint32_t prof_size;             // the size of the prof_tag fifo array
    FIFO *prof_tag;                // the profiled tags, stored in fifo structure
    uint32_t prof_tag_bits;         // the profiled tag bit length

    uint32_t *reuse_dis_cnt;        // reuse distance cnt (aka the reuse distance distribution)
    uint32_t reuse_cnt_width;       // sets within continuous #reuse_cnt_width will be cnted by the same reuse_dis_cnter


    uint64_t stage_size;            // the protection distance will be recomputed every #stage_size iterations
    uint64_t visit_cnt;             // the visitation counter

};


#endif // PD_H_
