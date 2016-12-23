#ifndef CPU_H
#define CPU_H

#include "global.h"

extern const unsigned char opcode_len[256];
extern const unsigned char opcode_ticks[256];
extern const char opcode_name[256][9];

class Cpu
{
private:
    WORD_16 accX;
    WORD_16 accD;
    ADDRESS pc;
    ADDRESS prev_pc;
    ADDRESS stackAddr;
    BYTE ccr;
    BYTE op;
    int wai_state;
    BYTE timer1_Store;
    bool OCR_waiting;
    bool NMI_waiting;
    int state;

    void illegal();
    void trap();
    void nop();
    void lsrd();
    void asld();
    void tap();
    void tpa();
    void inx();
    void dex();
    void clv();
    void sev();
    void clc();
    void sec();
    void cli();
    void sei();
    void sba();
    void cba();
    void tab();
    void tba();
    void xgdx();
    void daa();
    void slp();
    void aba();
    void bra(BYTE operand);
    void brn(BYTE operand);
    void bhi(BYTE operand);
    void bls(BYTE operand);
    void bcc(BYTE operand);
    void bcs(BYTE operand);
    void bne(BYTE operand);
    void beq(BYTE operand);
    void bvc(BYTE operand);
    void bvs(BYTE operand);
    void bpl(BYTE operand);
    void bmi(BYTE operand);
    void bge(BYTE operand);
    void blt(BYTE operand);
    void bgt(BYTE operand);
    void ble(BYTE operand);
    void tsx();
    void ins();
    void pula();
    void pulb();
    void des();
    void txs();
    void psha();
    void pshb();
    void pulx();
    void rts();
    void abx();
    void rti();
    void pshx();
    void mul();
    void wai();
    void swi(BYTE operand);
    void nega();
    void coma();
    void lsra();
    void rora();
    void asra();
    void asla();
    void rola();
    void deca();
    void inca();
    void tsta();
    void clra();
    void negb();
    void comb();
    void lsrb();
    void rorb();
    void asrb();
    void aslb();
    void rolb();
    void decb();
    void incb();
    void tstb();
    void clrb();
    void neg_ix(BYTE operand);
    void aim_ix(WORD_16 operand);
    void oim_ix(WORD_16 operand);
    void com_ix(BYTE operand);
    void lsr_ix(BYTE operand);
    void eim_ix(WORD_16 operand);
    void ror_ix(BYTE operand);
    void asr_ix(BYTE operand);
    void asl_ix(BYTE operand);
    void rol_ix(BYTE operand);
    void dec_ix(BYTE operand);
    void tim_ix(WORD_16 operand);
    void inc_ix(BYTE operand);
    void tst_ix(BYTE operand);
    void jmp_ix(BYTE operand);
    void clr_ix(BYTE operand);
    void neg_ex(WORD_16 operand);
    void aim_di(WORD_16 operand);
    void oim_di(WORD_16 operand);
    void com_ex(WORD_16 operand);
    void lsr_ex(WORD_16 operand);
    void eim_di(WORD_16 operand);
    void ror_ex(WORD_16 operand);
    void asr_ex(WORD_16 operand);
    void asl_ex(WORD_16 operand);
    void rol_ex(WORD_16 operand);
    void dec_ex(WORD_16 operand);
    void tim_di(WORD_16 operand);
    void inc_ex(WORD_16 operand);
    void tst_ex(WORD_16 operand);
    void jmp_ex(WORD_16 operand);
    void clr_ex(WORD_16 operand);
    void suba_im(BYTE operand);
    void cmpa_im(BYTE operand);
    void sbca_im(BYTE operand);
    void subd_im(WORD_16 operand);
    void anda_im(BYTE operand);
    void bita_im(BYTE operand);
    void lda_im(BYTE operand);
    void eora_im(BYTE operand);
    void adca_im(BYTE operand);
    void ora_im(BYTE operand);
    void adda_im(BYTE operand);
    void cpx_im(WORD_16 operand);
    void bsr(BYTE operand);
    void lds_im(WORD_16 operand);
    void suba_di(BYTE operand);
    void cmpa_di(BYTE operand);
    void sbca_di(BYTE operand);
    void subd_di(BYTE operand);
    void anda_di(BYTE operand);
    void bita_di(BYTE operand);
    void lda_di(BYTE operand);
    void sta_di(BYTE operand);
    void eora_di(BYTE operand);
    void adca_di(BYTE operand);
    void ora_di(BYTE operand);
    void adda_di(BYTE operand);
    void cpx_di(BYTE operand);
    void jsr_di(BYTE operand);
    void lds_di(BYTE operand);
    void sts_di(BYTE operand);
    void suba_ix(BYTE operand);
    void cmpa_ix(BYTE operand);
    void sbca_ix(BYTE operand);
    void subd_ix(BYTE operand);
    void anda_ix(BYTE operand);
    void bita_ix(BYTE operand);
    void lda_ix(BYTE operand);
    void sta_ix(BYTE operand);
    void eora_ix(BYTE operand);
    void adca_ix(BYTE operand);
    void ora_ix(BYTE operand);
    void adda_ix(BYTE operand);
    void cpx_ix(BYTE operand);
    void jsr_ix(BYTE operand);
    void lds_ix(BYTE operand);
    void sts_ix(BYTE operand);
    void suba_ex(WORD_16 operand);
    void cmpa_ex(WORD_16 operand);
    void sbca_ex(WORD_16 operand);
    void subd_ex(WORD_16 operand);
    void anda_ex(WORD_16 operand);
    void bita_ex(WORD_16 operand);
    void lda_ex(WORD_16 operand);
    void sta_ex(WORD_16 operand);
    void eora_ex(WORD_16 operand);
    void adca_ex(WORD_16 operand);
    void ora_ex(WORD_16 operand);
    void adda_ex(WORD_16 operand);
    void cpx_ex(WORD_16 operand);
    void jsr_ex(WORD_16 operand);
    void lds_ex(WORD_16 operand);
    void sts_ex(WORD_16 operand);
    void subb_im(BYTE operand);
    void cmpb_im(BYTE operand);
    void sbcb_im(BYTE operand);
    void addd_im(WORD_16 operand);
    void andb_im(BYTE operand);
    void bitb_im(BYTE operand);
    void ldb_im(BYTE operand);
    void eorb_im(BYTE operand);
    void adcb_im(BYTE operand);
    void orb_im(BYTE operand);
    void addb_im(BYTE operand);
    void ldd_im(WORD_16 operand);
    void ldx_im(WORD_16 operand);
    void subb_di(BYTE operand);
    void cmpb_di(BYTE operand);
    void sbcb_di(BYTE operand);
    void addd_di(BYTE operand);
    void andb_di(BYTE operand);
    void bitb_di(BYTE operand);
    void ldb_di(BYTE operand);
    void stb_di(BYTE operand);
    void eorb_di(BYTE operand);
    void adcb_di(BYTE operand);
    void orb_di(BYTE operand);
    void addb_di(BYTE operand);
    void ldd_di(BYTE operand);
    void std_di(BYTE operand);
    void ldx_di(BYTE operand);
    void stx_di(BYTE operand);
    void subb_ix(BYTE operand);
    void cmpb_ix(BYTE operand);
    void sbcb_ix(BYTE operand);
    void addd_ix(BYTE operand);
    void andb_ix(BYTE operand);
    void bitb_ix(BYTE operand);
    void ldb_ix(BYTE operand);
    void stb_ix(BYTE operand);
    void eorb_ix(BYTE operand);
    void adcb_ix(BYTE operand);
    void orb_ix(BYTE operand);
    void addb_ix(BYTE operand);
    void ldd_ix(BYTE operand);
    void std_ix(BYTE operand);
    void ldx_ix(BYTE operand);
    void stx_ix(BYTE operand);
    void subb_ex(WORD_16 operand);
    void cmpb_ex(WORD_16 operand);
    void sbcb_ex(WORD_16 operand);
    void addd_ex(WORD_16 operand);
    void andb_ex(WORD_16 operand);
    void bitb_ex(WORD_16 operand);
    void ldb_ex(WORD_16 operand);
    void stb_ex(WORD_16 operand);
    void eorb_ex(WORD_16 operand);
    void adcb_ex(WORD_16 operand);
    void orb_ex(WORD_16 operand);
    void addb_ex(WORD_16 operand);
    void ldd_ex(WORD_16 operand);
    void std_ex(WORD_16 operand);
    void ldx_ex(WORD_16 operand);
    void stx_ex(WORD_16 operand);

    void checkInterrupts();
    void doInterrupt(ADDRESS interrupt);
    void reset();
    int doOpcode();

public:
    Cpu();
    int doIteration();

    void notify_NMI();
    void setPower(bool power);
    void checkBits(BYTE data, BYTE mask);
    void W_timer1_CSR(BYTE data);
    BYTE R_timer1_FRC_H();
    void W_timer1_FRC_L(BYTE data);
    void W_timer1_FRC_H(BYTE data);
    void notify_timer1_OCR();
    void W_timer2_CSR(BYTE data);
};

extern Cpu *cpu;

#define accA accD.b.h
#define accB accD.b.l
#define SIGNED(b) ((SINT16)(b&0x80?b|0xff00:b))
#define BRANCH(f) {if(f){pc+=SIGNED(operand);}}

/******************************************/
/** Stack Control                        **/
/******************************************/
#define pullByte() memory->read(stackAddr+1); stackAddr++
#define pullWord() memory->read16(stackAddr+1); stackAddr+=2
#define pullAddr() memory->readAddress(stackAddr+1); stackAddr+=2;
#define pushByte(data) memory->write(stackAddr, data); stackAddr--
#define pushWord(data) memory->write16(stackAddr-1, data); stackAddr-=2
#define pushAddr(data) memory->writeAddress(stackAddr-1, data); stackAddr-=2;

/******************************************/
/** Condition Control Register           **/
/******************************************/
#define CLR_HNZVC	ccr&=0xd0
#define CLR_NZV 	ccr&=0xf1
#define CLR_HNZC	ccr&=0xd2
#define CLR_NZVC	ccr&=0xf0
#define CLR_Z		ccr&=0xfb
#define CLR_NZC 	ccr&=0xf2
#define CLR_ZC		ccr&=0xfa
#define CLR_C		ccr&=0xfe

#define SET_Z(a)		if(!(a))SEZ
#define SET_Z8(a)		SET_Z((UINT8)(a))
#define SET_Z16(a)		SET_Z((UINT16)(a))
#define SET_N8(a)		ccr|=(((a)&0x80)>>4)
#define SET_N16(a)		ccr|=(((a)&0x8000)>>12)
#define SET_H(a,b,r)	ccr|=((((a)^(b)^(r))&0x10)<<1)
#define SET_C8(a)		ccr|=(((a)&0x100)>>8)
#define SET_C16(a)		ccr|=(((a)&0x10000)>>16)
#define SET_V8(a,b,r)	ccr|=((((a)^(b)^(r)^((r)>>1))&0x80)>>6)
#define SET_V16(a,b,r)	ccr|=((((a)^(b)^(r)^((r)>>1))&0x8000)>>14)

#define NXORV  ((ccr&0x08)^((ccr&0x02)<<2))

static const UINT8 flags8i[256] =	 /* increment */
{
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0a, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};
static const UINT8 flags8d[256] = /* decrement */
{
    0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};
#define SET_FLAGS8I(a)		{ccr|=flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)		{ccr|=flags8d[(a)&0xff];}

#define SET_NZ8(a)			{SET_N8(a);SET_Z8(a);}
#define SET_NZ16(a)			{SET_N16(a);SET_Z16(a);}
#define SET_FLAGS8(a,b,r)	{SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)	{SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

#if defined(SEC)
#undef SEC
#endif
#define SEC ccr|=0x01
#define CLC ccr&=0xfe
#define SEZ ccr|=0x04
#define CLZ ccr&=0xfb
#define SEN ccr|=0x08
#define CLN ccr&=0xf7
#define SEV ccr|=0x02
#define CLV ccr&=0xfd
#define SEH ccr|=0x20
#define CLH ccr&=0xdf
#define SEI ccr|=0x10
#define CLI ccr&=~0x10

/******************************************/
/** CPU States                           **/
/******************************************/
#define CPU_Halt   0
#define CPU_Active 1
#define CPU_Reset  2
#define M6800_SLP	0x10
#define M6800_WAI	0x08

#endif // CPU_H
