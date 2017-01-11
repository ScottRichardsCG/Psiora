#include "debug.h"
#include "global.h"
#include "memory.h"
#include <stdio.h>
#include <string.h>
#ifdef Q_OS_WIN
#include <intrin.h>
#else
#include <signal.h>
#endif

#ifdef DEBUG_MODE_ENABLED
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

Debug *debug;

Debug::Debug()
{
    logFile = fopen("debug.log", "w+");
#ifdef DEBUG_MODE_ENABLED_CPU
	logFileCPU = fopen("debug_cpu.log", "w+");
	ignoreCmd = 0;
    //ignoreCmd = 47290; //CM Stop RAM check at 47290
    breakPoint = 0; //10000;
#endif
}

Debug::~Debug() {
	if (logFile)
		fclose(logFile);
#ifdef DEBUG_MODE_ENABLED_CPU
	if (logFileCPU)
		fclose(logFileCPU);
#endif
}

void Debug::writeOutput(char *log) {
    fprintf(stderr, "%s", log);
}

void Debug::writeLog(char *log) {
    if (logFile)
        fprintf(logFile, "%s\n", log);
}

#ifdef DEBUG_MODE_ENABLED_CPU
void Debug::writeCPU_1_0(ADDRESS pc, BYTE op) {
    if (ignoreCmd != 0) {
        return;
    }
    if logFileCPU) {
        char opname[9];
        char tmp[32];
        sprintf(tmp, "%s        ", opcode_name[op]);
        strncpy(opname, tmp, 8);
        opname[8] = '\0';
        fprintf(logFileCPU, "%04X> %s()     ", pc, opname);
    }
}

void Debug::writeCPU_1_8(ADDRESS pc, BYTE op, BYTE operand) {
    if (ignoreCmd != 0) {
        return;
    }
    if (logFileCPU) {
        char opname[9];
        char tmp[32];
        sprintf(tmp, "%s        ", opcode_name[op]);
        strncpy(opname, tmp, 8);
        opname[8] = '\0';
        fprintf(logFileCPU, "%04X> %s(%02X)   ", pc, opname, operand);
    }
}

void Debug::writeCPU_1_16(ADDRESS pc, BYTE op, WORD_16 operand) {
    if (ignoreCmd != 0) {
        return;
    }
    if (logFileCPU) {
        char opname[9];
        char tmp[32];
        sprintf(tmp, "%s        ", opcode_name[op]);
        strncpy(opname, tmp, 8);
        opname[8] = '\0';
        fprintf(logFileCPU, "%04X> %s(%04X) ", pc, opname, operand.w);
    }
}

void Debug::writeCPU_2(WORD_16 D, WORD_16 X, ADDRESS SP, BYTE CCR) {
    if (ignoreCmd != 0) {
        ignoreCmd--;
        return;
    }
    if (breakPoint != 0) {
        breakPoint--;
        if (breakPoint == 0) {
           char memTmp[65536];
           int i;
           for (i=0; i<65536; i++) {
               memTmp[i] = memory->readDirect(i);
           }
#ifdef Q_OS_WIN
            __debugbreak();
#else
           raise(SIGABRT);
#endif
           memTmp[0]++; // To stop unused variable warnings
        }
    }
    if (logFileCPU) {
        char debug_CCR[9];
        debug_CCR[0] = ((CCR & 0x80) / 0x80) * 49;
        debug_CCR[1] = ((CCR & 0x40) / 0x40) * 49;
        debug_CCR[2] = ((CCR & 0x20) / 0x20) * 72;
        debug_CCR[3] = ((CCR & 0x10) / 0x10) * 73;
        debug_CCR[4] = ((CCR & 0x08) / 0x08) * 78;
        debug_CCR[5] = ((CCR & 0x04) / 0x04) * 90;
        debug_CCR[6] = ((CCR & 0x02) / 0x02) * 86;
        debug_CCR[7] = ((CCR & 0x01) / 0x01) * 67;
        debug_CCR[8] = '\0';
        int a;
        for (a = 0; a<8; a++) {
            if (debug_CCR[a] == 0)
                debug_CCR[a] = 45;
        }

        fprintf(logFileCPU, "%04X %04X %04X %s\n", D.w, X.w, SP, debug_CCR);
    }
}
#endif
#endif
