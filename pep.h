#include <limits.h>
#include <stdint.h>

static char MEMORY[USHRT_MAX+1];
enum MODE {
        MODE_I,// immediate oprd
        MODE_D,// direct Mem[oprd]
        MODE_N,// indirect Mem[Mem[oprd]]
        MODE_X,// x addressed Mem[oprd + X]
        MODE_S,// stack relative Mem[SP + oprd]
        MODE_SF,// stack relative indirect Mem[Mem[SP+oprd]]
        MODE_SX,// stack relative x addressed Mem[SP + oprd + X]
        MODE_SXF,// stack relative x addressed indirect Mem[Mem[SP+oprd] + X]
};

enum REG_ID {
        REG_A_ID,
        REG_X_ID,
        REG_SP_ID,
};


#define REGA_ID 0
#define REGX_ID 1
#define REGSP_ID 2

static int16_t REG_A;
static int16_t REG_X;

static int16_t REG_SP;
static int16_t REG_PC;

#define FLGM_N 0x08
#define FLGM_Z 0x04
#define FLGM_V 0x02
#define FLGM_C 0x01
static char REG_NZVC;

void OP_ADD(MODE reg_id, int16_t arg, char mode);
