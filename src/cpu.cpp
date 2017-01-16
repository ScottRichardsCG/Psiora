#include "global.h"
#include "cpu.h"
#include "memory.h"
#include "debug.h"
#include "datapak.h"
#include <cstdio>

#define callOp(number, name, parameter) case number: name(parameter); break;

#define TIMER1_CSR			0x0008
#define TIMER1_FRC			0x0009
#define TIMER1_FRC_h		0x0009
#define TIMER1_FRC_l		0x000A
#define TIMER1_OCR			0x000B
#define TIMER1_OCR_h		0x000B
#define TIMER1_OCR_l		0x000C
#define TIMER2_CSR			0x000F
#define INTERRUPT_OCR		0xFFF4
#define INTERRUPT_NMI		0xFFFC
#define INTERRUPT_BOOT	0xFFFE

Cpu *cpu;

// OP-CODE INFORMATION
const unsigned char opcode_len[256] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 3, 3, 2, 2, 3, 2, 2, 2,
    2, 2, 3, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 2, 2, 2, 3, 2, 2, 2, 1, 2, 2, 2, 2, 3, 2, 3, 1, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 2, 2, 2, 3, 2, 2, 2, 1, 2, 2, 2, 2, 3, 1, 3, 1, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3
};

const unsigned char opcode_ticks[256] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 2, 2, 4, 1, 1, 1, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 1, 1, 3, 3, 1, 1, 4, 4, 4, 5, 1, 10, 5, 7, 9,
    12, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 7, 7, 6, 6, 7, 6, 6, 6,
    6, 6, 5, 6, 4, 3, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 4, 6, 4,
    3, 5, 2, 2, 2, 3, 2, 2, 2, 1, 2, 2, 2, 2, 3, 5, 3, 1, 3, 3, 3,
    4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 5, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4,
    4, 4, 4, 4, 5, 5, 5, 5, 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4, 5,
    6, 5, 5, 2, 2, 2, 3, 2, 2, 2, 1, 2, 2, 2, 2, 3, 1, 3, 1, 3, 3,
    3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4,
    4, 4, 4, 4, 4, 5, 5, 5, 5, 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5
};

const char opcode_name[256][9] = {
    "trap", "nop", "illegal", "illegal", "lsrd", "asld", "tap", "tpa",
    "inx", "dex", "clv", "sev", "clc", "sec", "cli", "sei",
    "sba", "cba", "illegal", "illegal", "illegal", "illegal", "tab", "tba",
    "xgdx", "daa", "slp", "aba", "illegal", "illegal", "illegal", "illegal",
    "bra", "brn", "bhi", "bls", "bcc", "bcs", "bne", "beq",
    "bvc", "bvs", "bpl", "bmi", "bge", "blt", "bgt", "ble",
    "tsx", "ins", "pula", "pulb", "des", "txs", "psha", "pshb",
    "pulx", "rts", "abx", "rti", "pshx", "mul", "wai", "swi",
    "nega", "illegal", "illegal", "coma", "lsra", "illegal", "rora", "asra",
    "asla", "rola", "deca", "illegal", "inca", "tsta", "illegal", "clra",
    "negb", "illegal", "illegal", "comb", "lsrb", "illegal", "rorb", "asrb",
    "aslb", "rolb", "decb", "illegal", "incb", "tstb", "illegal", "clrb",
    "neg_ix", "aim_ix", "oim_ix", "com_ix", "lsr_ix", "eim_ix", "ror_ix", "asr_ix",
    "asl_ix", "rol_ix", "dec_ix", "tim_ix", "inc_ix", "tst_ix", "jmp_ix", "clr_ix",
    "neg_ex", "aim_di", "oim_di", "com_ex", "lsr_ex", "eim_di", "ror_ex", "asr_ex",
    "asl_ex", "rol_ex", "dec_ex", "tim_di", "inc_ex", "tst_ex", "jmp_ex", "clr_ex",
    "suba_im", "cmpa_im", "sbca_im", "subd_im", "anda_im", "bita_im", "lda_im", "illegal",
    "eora_im", "adca_im", "ora_im", "adda_im", "cpx_im", "bsr", "lds_im", "illegal",
    "suba_di", "cmpa_di", "sbca_di", "subd_di", "anda_di", "bita_di", "lda_di", "sta_di",
    "eora_di", "adca_di", "ora_di", "adda_di", "cpx_di", "jsr_di", "lds_di", "sts_di",
    "suba_ix", "cmpa_ix", "sbca_ix", "subd_ix", "anda_ix", "bita_ix", "lda_ix", "sta_ix",
    "eora_ix", "adca_ix", "ora_ix", "adda_ix", "cpx_ix", "jsr_ix", "lds_ix", "sts_ix",
    "suba_ex", "cmpa_ex", "sbca_ex", "subd_ex", "anda_ex", "bita_ex", "lda_ex", "sta_ex",
    "eora_ex", "adca_ex", "ora_ex", "adda_ex", "cpx_ex", "jsr_ex", "lds_ex", "sts_ex",
    "subb_im", "cmpb_im", "sbcb_im", "addd_im", "andb_im", "bitb_im", "ldb_im", "illegal",
    "eorb_im", "adcb_im", "orb_im", "addb_im", "ldd_im", "illegal", "ldx_im", "illegal",
    "subb_di", "cmpb_di", "sbcb_di", "addd_di", "andb_di", "bitb_di", "ldb_di", "stb_di",
    "eorb_di", "adcb_di", "orb_di", "addb_di", "ldd_di", "std_di", "ldx_di", "stx_di",
    "subb_ix", "cmpb_ix", "sbcb_ix", "addd_ix", "andb_ix", "bitb_ix", "ldb_ix", "stb_ix",
    "eorb_ix", "adcb_ix", "orb_ix", "addb_ix", "ldd_ix", "std_ix", "ldx_ix", "stx_ix",
    "subb_ex", "cmpb_ex", "sbcb_ex", "addd_ex", "andb_ex", "bitb_ex", "ldb_ex", "stb_ex",
    "eorb_ex", "adcb_ex", "orb_ex", "addb_ex", "ldd_ex", "std_ex", "ldx_ex", "stx_ex"
};

Cpu::Cpu()
{
}

void Cpu::setPower(bool power)
{
    if (!power) state = CPU_Halt;
    else state = CPU_Reset;
}

int Cpu::doIteration()
{
    int cycles;
    switch (state)
    {
    case CPU_Reset:
        reset();
        return 1;
    case CPU_Halt:
        return 9216;
    case CPU_Active:
        if ((NMI_waiting || OCR_waiting) && (!(ccr & 0x10))) {
            checkInterrupts();
            cycles = 1;
        } else if (wai_state & (M6800_WAI | M6800_SLP)) {
            cycles = 1;
        } else {
            cycles = doOpcode();
        }

        if (memory->readDirect(TIMER1_CSR) & 0x08)
        {
            WORD_16 tmp1, tmp2;
            tmp1 = memory->readDirect16(TIMER1_FRC);
            tmp2 = memory->readDirect16(TIMER1_OCR);

            tmp1.w += cycles;
            if (tmp1.w >= tmp2.w) {
                tmp1.w -= tmp2.w;
                OCR_waiting = true;
            }

            memory->writeDirect16(TIMER1_FRC, tmp1);
        }
        return cycles;
    }
    return 0;
}

void Cpu::checkBits(BYTE data, BYTE mask)
{
    if (data & mask)
    {
        // Problem with CPU bits
        // Useful for CPU debugging
        // Used to be attached to the HardwareError LCD code
    }
}

void Cpu::notify_NMI()
{
    NMI_waiting = true;
}

void Cpu::W_timer1_CSR(BYTE data)
{
    memory->writeDirect(TIMER1_CSR, ((data & 0x1f) + (memory->readDirect(TIMER1_CSR) & 0xd0)));
}

BYTE Cpu::R_timer1_FRC_H()
{
    memory->writeDirect(TIMER1_CSR, (memory->readDirect(TIMER1_CSR) & 0xdf));
    return memory->readDirect(TIMER1_FRC_h);
}

void Cpu::W_timer1_FRC_H(BYTE data)
{
    timer1_Store = data;
    memory->writeDirect(TIMER1_FRC_h, 0xff);
    memory->writeDirect(TIMER1_FRC_l, 0xf8);
}

void Cpu::W_timer1_FRC_L(BYTE data)
{
    memory->writeDirect(TIMER1_FRC_h, timer1_Store);
    memory->writeDirect(TIMER1_FRC_l, data);
}

void Cpu::notify_timer1_OCR()
{
    memory->writeDirect(TIMER1_OCR_h, memory->readDirect(TIMER1_OCR_h) & 0xbf);
    // Do this to timer2 too????
}

void Cpu::W_timer2_CSR(BYTE data)
{
    checkBits(data, 0x0b);
    memory->writeDirect(TIMER2_CSR, ((data & 0x0f) + 0x10 + (memory->readDirect(TIMER2_CSR) & 0xd0)));
}

void Cpu::checkInterrupts()
{
    if (OCR_waiting)
    {
        if (wai_state & M6800_SLP)
            wai_state &= ~M6800_SLP;
        OCR_waiting = false;
        doInterrupt(INTERRUPT_OCR);
    }
    else if (NMI_waiting)
    {
        if (wai_state & M6800_SLP)
            wai_state &= ~M6800_SLP;
        NMI_waiting = false;
        doInterrupt(INTERRUPT_NMI);
    }
}

void Cpu::doInterrupt(ADDRESS interrupt)
{
    if (wai_state & (M6800_WAI | M6800_SLP))
    {
        wai_state &= ~(M6800_WAI | M6800_SLP);
    }
    else
    {
        pushAddr(pc);
        pushWord(accX);
        pushByte(accD.b.h);
        pushByte(accD.b.l);
        pushByte(ccr);
    }
    SEI;
    pc = memory->readAddress(interrupt);
}

void Cpu::reset()
{
		// Reset CPU Registers
		
	memory->write(0x0001, 0xFC);
    memory->writeDirect(0x0008, 0x00);
    memory->writeDirect(0x0009, 0x00);
    memory->writeDirect(0x000a, 0x00);
    memory->writeDirect(0x000b, 0xff);
    memory->writeDirect(0x000c, 0xff);
    memory->writeDirect(0x000d, 0x00);
    memory->writeDirect(0x000e, 0x00);
    memory->writeDirect(0x000f, 0x10);
    memory->writeDirect(0x0010, 0x00);
    memory->writeDirect(0x0011, 0x20);
    memory->writeDirect(0x0012, 0x00);
    memory->writeDirect(0x0013, 0x00);
    BYTE ram = memory->readDirect(0x0014);
	ram |= 0x7C;
    memory->writeDirect(0x0014, ram);
	memory->write(0x0016, 0x00);
    memory->writeDirect(0x0019, 0xFF);
    memory->writeDirect(0x001A, 0xFF);
    memory->writeDirect(0x001B, 0x20);
    memory->writeDirect(0x001C, 0xFF);
    memory->writeDirect(0x001D, 0x00);


    accD.w = 0;
    accX.w = 0;
    stackAddr = 0;
    ccr = 0xc0;
    NMI_waiting = false;
    OCR_waiting = false;
    wai_state = 0;
    state = CPU_Active;

    WORD_16 tmp = memory->read16(INTERRUPT_BOOT);
    pc = tmp.w;
}

int Cpu::doOpcode()
{
    BYTE tmpb = 0;
    WORD_16 tmpw;

    prev_pc = pc;
    op = memory->read(pc);
    pc++;
    if (opcode_len[op] == 2)
    {
        tmpb = memory->read(pc);
        pc++;
#ifdef DEBUG_MODE_ENABLED_CPU
        debug->writeCPU_1_8(prev_pc, op, tmpb);
#endif
    }
    else if (opcode_len[op] == 3)
    {
        tmpw = memory->read16(pc);
        pc+=2;
#ifdef DEBUG_MODE_ENABLED_CPU
        debug->writeCPU_1_16(prev_pc, op, tmpw);
#endif
    }
    else {
#ifdef DEBUG_MODE_ENABLED_CPU
        debug->writeCPU_1_0(prev_pc, op);
#endif
    }

    switch (op)
    {
    callOp(0x00, trap, );
    callOp(0x01, nop, );
    callOp(0x02, illegal, );
    callOp(0x03, illegal, );
    callOp(0x04, lsrd, );
    callOp(0x05, asld, );
    callOp(0x06, tap, );
    callOp(0x07, tpa, );
    callOp(0x08, inx, );
    callOp(0x09, dex, );
    callOp(0x0A, clv, );
    callOp(0x0B, sev, );
    callOp(0x0C, clc, );
    callOp(0x0D, sec, );
    callOp(0x0E, cli, );
    callOp(0x0F, sei, );
    callOp(0x10, sba, );
    callOp(0x11, cba, );
    callOp(0x12, illegal, );
    callOp(0x13, illegal, );
    callOp(0x14, illegal, );
    callOp(0x15, illegal, );
    callOp(0x16, tab, );
    callOp(0x17, tba, );
    callOp(0x18, xgdx, );
    callOp(0x19, daa, );
    callOp(0x1A, slp, );
    callOp(0x1B, aba, );
    callOp(0x1C, illegal, );
    callOp(0x1D, illegal, );
    callOp(0x1E, illegal, );
    callOp(0x1F, illegal, );
    callOp(0x20, bra, tmpb);
    callOp(0x21, brn, tmpb);
    callOp(0x22, bhi, tmpb);
    callOp(0x23, bls, tmpb);
    callOp(0x24, bcc, tmpb);
    callOp(0x25, bcs, tmpb);
    callOp(0x26, bne, tmpb);
    callOp(0x27, beq, tmpb);
    callOp(0x28, bvc, tmpb);
    callOp(0x29, bvs, tmpb);
    callOp(0x2A, bpl, tmpb);
    callOp(0x2B, bmi, tmpb);
    callOp(0x2C, bge, tmpb);
    callOp(0x2D, blt, tmpb);
    callOp(0x2E, bgt, tmpb);
    callOp(0x2F, ble, tmpb);
    callOp(0x30, tsx, );
    callOp(0x31, ins, );
    callOp(0x32, pula, );
    callOp(0x33, pulb, );
    callOp(0x34, des, );
    callOp(0x35, txs, );
    callOp(0x36, psha, );
    callOp(0x37, pshb, );
    callOp(0x38, pulx, );
    callOp(0x39, rts, );
    callOp(0x3A, abx, );
    callOp(0x3B, rti, );
    callOp(0x3C, pshx, );
    callOp(0x3D, mul, );
    callOp(0x3E, wai, );
    callOp(0x3F, swi, tmpb);
    callOp(0x40, nega, );
    callOp(0x41, illegal, );
    callOp(0x42, illegal, );
    callOp(0x43, coma, );
    callOp(0x44, lsra, );
    callOp(0x45, illegal, );
    callOp(0x46, rora, );
    callOp(0x47, asra, );
    callOp(0x48, asla, );
    callOp(0x49, rola, );
    callOp(0x4A, deca, );
    callOp(0x4B, illegal, );
    callOp(0x4C, inca, );
    callOp(0x4D, tsta, );
    callOp(0x4E, illegal, );
    callOp(0x4F, clra, );
    callOp(0x50, negb, );
    callOp(0x51, illegal, );
    callOp(0x52, illegal, );
    callOp(0x53, comb, );
    callOp(0x54, lsrb, );
    callOp(0x55, illegal, );
    callOp(0x56, rorb, );
    callOp(0x57, asrb, );
    callOp(0x58, aslb, );
    callOp(0x59, rolb, );
    callOp(0x5A, decb, );
    callOp(0x5B, illegal, );
    callOp(0x5C, incb, );
    callOp(0x5D, tstb, );
    callOp(0x5E, illegal, );
    callOp(0x5F, clrb, );
    callOp(0x60, neg_ix, tmpb);
    callOp(0x61, aim_ix, tmpw);
    callOp(0x62, oim_ix, tmpw);
    callOp(0x63, com_ix, tmpb);
    callOp(0x64, lsr_ix, tmpb);
    callOp(0x65, eim_ix, tmpw);
    callOp(0x66, ror_ix, tmpb);
    callOp(0x67, asr_ix, tmpb);
    callOp(0x68, asl_ix, tmpb);
    callOp(0x69, rol_ix, tmpb);
    callOp(0x6A, dec_ix, tmpb);
    callOp(0x6B, tim_ix, tmpw);
    callOp(0x6C, inc_ix, tmpb);
    callOp(0x6D, tst_ix, tmpb);
    callOp(0x6E, jmp_ix, tmpb);
    callOp(0x6F, clr_ix, tmpb);
    callOp(0x70, neg_ex, tmpw);
    callOp(0x71, aim_di, tmpw);
    callOp(0x72, oim_di, tmpw);
    callOp(0x73, com_ex, tmpw);
    callOp(0x74, lsr_ex, tmpw);
    callOp(0x75, eim_di, tmpw);
    callOp(0x76, ror_ex, tmpw);
    callOp(0x77, asr_ex, tmpw);
    callOp(0x78, asl_ex, tmpw);
    callOp(0x79, rol_ex, tmpw);
    callOp(0x7A, dec_ex, tmpw);
    callOp(0x7B, tim_di, tmpw);
    callOp(0x7C, inc_ex, tmpw);
    callOp(0x7D, tst_ex, tmpw);
    callOp(0x7E, jmp_ex, tmpw);
    callOp(0x7F, clr_ex, tmpw);
    callOp(0x80, suba_im, tmpb);
    callOp(0x81, cmpa_im, tmpb);
    callOp(0x82, sbca_im, tmpb);
    callOp(0x83, subd_im, tmpw);
    callOp(0x84, anda_im, tmpb);
    callOp(0x85, bita_im, tmpb);
    callOp(0x86, lda_im, tmpb);
    callOp(0x87, illegal, );
    callOp(0x88, eora_im, tmpb);
    callOp(0x89, adca_im, tmpb);
    callOp(0x8A, ora_im, tmpb);
    callOp(0x8B, adda_im, tmpb);
    callOp(0x8C, cpx_im, tmpw);
    callOp(0x8D, bsr, tmpb);
    callOp(0x8E, lds_im, tmpw);
    callOp(0x8F, illegal, );
    callOp(0x90, suba_di, tmpb);
    callOp(0x91, cmpa_di, tmpb);
    callOp(0x92, sbca_di, tmpb);
    callOp(0x93, subd_di, tmpb);
    callOp(0x94, anda_di, tmpb);
    callOp(0x95, bita_di, tmpb);
    callOp(0x96, lda_di, tmpb);
    callOp(0x97, sta_di, tmpb);
    callOp(0x98, eora_di, tmpb);
    callOp(0x99, adca_di, tmpb);
    callOp(0x9A, ora_di, tmpb);
    callOp(0x9B, adda_di, tmpb);
    callOp(0x9C, cpx_di, tmpb);
    callOp(0x9D, jsr_di, tmpb);
    callOp(0x9E, lds_di, tmpb);
    callOp(0x9F, sts_di, tmpb);
    callOp(0xA0, suba_ix, tmpb);
    callOp(0xA1, cmpa_ix, tmpb);
    callOp(0xA2, sbca_ix, tmpb);
    callOp(0xA3, subd_ix, tmpb);
    callOp(0xA4, anda_ix, tmpb);
    callOp(0xA5, bita_ix, tmpb);
    callOp(0xA6, lda_ix, tmpb);
    callOp(0xA7, sta_ix, tmpb);
    callOp(0xA8, eora_ix, tmpb);
    callOp(0xA9, adca_ix, tmpb);
    callOp(0xAA, ora_ix, tmpb);
    callOp(0xAB, adda_ix, tmpb);
    callOp(0xAC, cpx_ix, tmpb);
    callOp(0xAD, jsr_ix, tmpb);
    callOp(0xAE, lds_ix, tmpb);
    callOp(0xAF, sts_ix, tmpb);
    callOp(0xB0, suba_ex, tmpw);
    callOp(0xB1, cmpa_ex, tmpw);
    callOp(0xB2, sbca_ex, tmpw);
    callOp(0xB3, subd_ex, tmpw);
    callOp(0xB4, anda_ex, tmpw);
    callOp(0xB5, bita_ex, tmpw);
    callOp(0xB6, lda_ex, tmpw);
    callOp(0xB7, sta_ex, tmpw);
    callOp(0xB8, eora_ex, tmpw);
    callOp(0xB9, adca_ex, tmpw);
    callOp(0xBA, ora_ex, tmpw);
    callOp(0xBB, adda_ex, tmpw);
    callOp(0xBC, cpx_ex, tmpw);
    callOp(0xBD, jsr_ex, tmpw);
    callOp(0xBE, lds_ex, tmpw);
    callOp(0xBF, sts_ex, tmpw);
    callOp(0xC0, subb_im, tmpb);
    callOp(0xC1, cmpb_im, tmpb);
    callOp(0xC2, sbcb_im, tmpb);
    callOp(0xC3, addd_im, tmpw);
    callOp(0xC4, andb_im, tmpb);
    callOp(0xC5, bitb_im, tmpb);
    callOp(0xC6, ldb_im, tmpb);
    callOp(0xC7, illegal, );
    callOp(0xC8, eorb_im, tmpb);
    callOp(0xC9, adcb_im, tmpb);
    callOp(0xCA, orb_im, tmpb);
    callOp(0xCB, addb_im, tmpb);
    callOp(0xCC, ldd_im, tmpw);
    callOp(0xCD, illegal, );
    callOp(0xCE, ldx_im, tmpw);
    callOp(0xCF, illegal, );
    callOp(0xD0, subb_di, tmpb);
    callOp(0xD1, cmpb_di, tmpb);
    callOp(0xD2, sbcb_di, tmpb);
    callOp(0xD3, addd_di, tmpb);
    callOp(0xD4, andb_di, tmpb);
    callOp(0xD5, bitb_di, tmpb);
    callOp(0xD6, ldb_di, tmpb);
    callOp(0xD7, stb_di, tmpb);
    callOp(0xD8, eorb_di, tmpb);
    callOp(0xD9, adcb_di, tmpb);
    callOp(0xDA, orb_di, tmpb);
    callOp(0xDB, addb_di, tmpb);
    callOp(0xDC, ldd_di, tmpb);
    callOp(0xDD, std_di, tmpb);
    callOp(0xDE, ldx_di, tmpb);
    callOp(0xDF, stx_di, tmpb);
    callOp(0xE0, subb_ix, tmpb);
    callOp(0xE1, cmpb_ix, tmpb);
    callOp(0xE2, sbcb_ix, tmpb);
    callOp(0xE3, addd_ix, tmpb);
    callOp(0xE4, andb_ix, tmpb);
    callOp(0xE5, bitb_ix, tmpb);
    callOp(0xE6, ldb_ix, tmpb);
    callOp(0xE7, stb_ix, tmpb);
    callOp(0xE8, eorb_ix, tmpb);
    callOp(0xE9, adcb_ix, tmpb);
    callOp(0xEA, orb_ix, tmpb);
    callOp(0xEB, addb_ix, tmpb);
    callOp(0xEC, ldd_ix, tmpb);
    callOp(0xED, std_ix, tmpb);
    callOp(0xEE, ldx_ix, tmpb);
    callOp(0xEF, stx_ix, tmpb);
    callOp(0xF0, subb_ex, tmpw);
    callOp(0xF1, cmpb_ex, tmpw);
    callOp(0xF2, sbcb_ex, tmpw);
    callOp(0xF3, addd_ex, tmpw);
    callOp(0xF4, andb_ex, tmpw);
    callOp(0xF5, bitb_ex, tmpw);
    callOp(0xF6, ldb_ex, tmpw);
    callOp(0xF7, stb_ex, tmpw);
    callOp(0xF8, eorb_ex, tmpw);
    callOp(0xF9, adcb_ex, tmpw);
    callOp(0xFA, orb_ex, tmpw);
    callOp(0xFB, addb_ex, tmpw);
    callOp(0xFC, ldd_ex, tmpw);
    callOp(0xFD, std_ex, tmpw);
    callOp(0xFE, ldx_ex, tmpw);
    callOp(0xFF, stx_ex, tmpw);
    }

#ifdef DEBUG_MODE_ENABLED_CPU
    debug->writeCPU_2(accD, accX, stackAddr, ccr);
#endif

    return opcode_ticks[op];
}
