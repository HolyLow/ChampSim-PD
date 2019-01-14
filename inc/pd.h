#ifndef PD_H_
 #define PD_H_

#include "cache.h"

#define PD_DEBUG_FLAG
#define PD_LOG_FLAG

#ifdef PD_DEBUG_FLAG 
 #define PD_DEBUG(format, ...) \
  do { \
    printf("==DEBUG>> <%s : %d : %s>: " format "\n", \
        __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); \
  } while(0)
#else 
 #define PD_DEBUG(format, ...)            
#endif 

#ifdef PD_LOG_FLAG 
 #define PD_LOG(format, ...) \
  do { \
    printf("===LOG>>> <%s : %d : %s>: " format "\n", \
        __FILE__, __LINE__, __FUNCTION__, ## __VA_ARGS__); \
  } while(0) 
#else        
 #define PD_LOG(format, ...)  
#endif 

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

// define policies in the flag mode(1, 2, 4, 8, ...) to turn in / out the option
#define PD_ORIGINAL         1
#define PD_SOFT_UPDATE      2
#define PD_VICTIM           4
#define PD_MAX              8


class PD {
public:
    PD(BLOCK** blk, uint32_t p = PD_ORIGINAL, uint32_t prof_siz = 32, uint32_t reuse_cnt_wid = 4);
    ~PD();
    void update(uint32_t set, uint32_t way);
    uint32_t victim(uint32_t cpu, uint64_t instr_id, uint32_t set, 
        const BLOCK *current_set, uint64_t ip, uint64_t full_addr, uint32_t type);

private:
    void update_protection_distance();
    void update_reuse_dis_counter(uint32_t reuse_dis);

    BLOCK ** block;

    uint32_t policy;                // the pd_related policy

    uint32_t total_set;             // llc cache set number
    uint32_t prof_set;              // profiled set number
    uint32_t total_way;             // llc cache way number, also indicates the set associativity
    
    uint32_t max_dis;               // max profiled distance
    uint32_t prt_dis;               // protection distance
    uint32_t **remain_prt_dis;      // remained protection distance for all the sets
    bool **used_flag;               // flag for each ways, indicate if the way has been used
    bool **victim_flag;

    uint32_t prof_step;             // step of profiling, only one visit among #prof_step visits will be inserted to the prof_tag
    uint32_t* prof_step_cnt;        // cnter of profiling step for each prof_set
    uint32_t prof_size;             // the size of the prof_tag fifo array
    FIFO *prof_tag;                // the profiled tags, stored in fifo structure
    uint32_t prof_tag_bits;         // the profiled tag bit length

    uint32_t *reuse_dis_cnt;        // reuse distance cnt (aka the reuse distance distribution)
    uint32_t reuse_cnt_width;       // sets within continuous #reuse_cnt_width will be cnted by the same reuse_dis_cnter


    uint64_t stage_size;            // the protection distance will be recomputed every #stage_size iterations
    uint64_t visit_cnt;             // the visitation counter
    uint64_t prof_cnt;              // the profiled visitation counter

    uint64_t invalid_vic_cnt;
    uint64_t prt_zero_vic_cnt;
    uint64_t unreused_vic_cnt;
    uint64_t reused_vic_cnt;

};


#endif // PD_H_
