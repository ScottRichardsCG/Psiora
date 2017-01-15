#include "global.h"
#include "cpu.h"
#include "memory.h"

/*** Interrupts ***/
void Cpu::illegal() {
    pushAddr(prev_pc);
    pushWord(accX);
    pushByte(accA);
    pushByte(accB);
    pushByte(ccr);
    SEI;
    pc = memory->readAddress(0xffee);
}
void Cpu::trap() {
    pushAddr(prev_pc);
    pushWord(accX);
    pushByte(accA);
    pushByte(accB);
    pushByte(ccr);
    SEI;
    pc = memory->readAddress(0xffee);
}
void Cpu::wai() {
    wai_state |= M6800_WAI;
    pushAddr(pc);
    pushWord(accX);
    pushByte(accA);
    pushByte(accB);
    pushByte(ccr);
    checkInterrupts();
}
void Cpu::swi(BYTE) {
    pushAddr(pc);
    pushWord(accX);
    pushByte(accA);
    pushByte(accB);
    pushByte(ccr);
    SEI;
    pc = memory->readAddress(0xfffa);
}

/*** Return from routine ***/
void Cpu::rti() {
    ccr = pullByte();
    accB = pullByte();
    accA = pullByte();
    accX = pullWord();
    pc = pullAddr();
}
void Cpu::rts() { pc = pullAddr(); }

/*** General Instructions ***/
void Cpu::nop() {
    // Do Nothing
}
void Cpu::slp() {
    wai_state |= M6800_SLP;
}

/*** Accumulator swapping ***/
void Cpu::tab() {
    accB = accA;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::tba() {
    accA = accB;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::xgdx() {
    WORD_16 t = accX;
    accX = accD;
    accD = t;
}

/*** Condition Control Register ***/
void Cpu::tap() { ccr = accD.b.h; }
void Cpu::tpa() { accD.b.h = ccr; }
void Cpu::clv() { CLV; }
void Cpu::sev() { SEV; }
void Cpu::clc() { CLC; }
void Cpu::sec() { SEC; }
void Cpu::cli() { CLI; }
void Cpu::sei() { SEI; }

/*** Branches ***/
void Cpu::bra(BYTE operand) { pc += SIGNED(operand); }
void Cpu::brn(BYTE) { pc++; }
void Cpu::bhi(BYTE operand) { BRANCH(!(ccr & 0x05)); }
void Cpu::bls(BYTE operand) { BRANCH(ccr & 0x05); }
void Cpu::bcc(BYTE operand) { BRANCH(!(ccr & 0x01)); }
void Cpu::bcs(BYTE operand) { BRANCH(ccr & 0x01); }
void Cpu::bne(BYTE operand) { BRANCH(!(ccr & 0x04)); }
void Cpu::beq(BYTE operand) { BRANCH(ccr & 0x04); }
void Cpu::bvc(BYTE operand) { BRANCH(!(ccr & 0x02)); }
void Cpu::bvs(BYTE operand) { BRANCH(ccr & 0x02); }
void Cpu::bpl(BYTE operand) { BRANCH(!(ccr & 0x08)); }
void Cpu::bmi(BYTE operand) { BRANCH(ccr & 0x08); }
void Cpu::bge(BYTE operand) { BRANCH(!NXORV); }
void Cpu::blt(BYTE operand) { BRANCH(NXORV); }
void Cpu::bgt(BYTE operand) { BRANCH(!(NXORV || ccr & 0x04)); }
void Cpu::ble(BYTE operand) { BRANCH(NXORV || ccr & 0x04); }
void Cpu::bsr(BYTE operand) {
    pushAddr(pc);
    pc += SIGNED(operand);
}

/*** Misc Mathematical Functions ***/
void Cpu::daa() {
    UINT8 msn, lsn;
    UINT16 t, cf = 0;
    msn = accA & 0xf0; lsn = accA & 0x0f;
    if (lsn > 0x09 || ccr & 0x20) cf |= 0x06;
    if (msn > 0x80 && lsn > 0x09) cf |= 0x60;
    if (msn > 0x90 || ccr & 0x01) cf |= 0x60;
    t = cf + accA;
    CLR_NZV;
    SET_N8((UINT8)t); SET_C8(t);
    accA = (BYTE)t;
}
void Cpu::mul() {
    UINT16 t;
    t = accA * accB;
    CLR_C;
    if (t & 0x80) SEC;
    accD.w = t;
}

/*** Stack Control ***/
void Cpu::tsx() { accX.w = stackAddr + 1; }
void Cpu::txs() { stackAddr = accX.w - 1; }
void Cpu::pula() { accA = pullByte(); }
void Cpu::pulb() { accB = pullByte(); }
void Cpu::pulx() { accX = pullWord(); }
void Cpu::psha() { pushByte(accA); }
void Cpu::pshb() { pushByte(accB); }
void Cpu::pshx() { pushWord(accX); }

/*** Logically Shift Right ***/
void Cpu::lsrd() {
    UINT16 t;
    CLR_NZC; t = accD.w; ccr |= (t & 0x0001);
    t >>= 1; SET_Z16(t); accD.w = t;
}
void Cpu::lsra() {
    CLR_NZC; ccr |= (accA & 0x01);
    accA >>= 1; SET_Z8(accA);
}
void Cpu::lsrb() {
    CLR_NZC; ccr |= (accB & 0x01);
    accB >>= 1; SET_Z8(accB);
}
void Cpu::lsr_ix(BYTE operand) {
    BYTE t = memory->read(accX.w + operand);
    CLR_NZC; ccr |= (t & 0x01);
    t >>= 1; SET_Z8(t);
    memory->write(accX.w + operand, t);
}
void Cpu::lsr_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    CLR_NZC; ccr |= (t & 0x01);
    t >>= 1; SET_Z8(t);
    memory->write(operand.w, t);
}

/*** Rotate Right Through Carrier ***/
void Cpu::rora() {
    BYTE r = (ccr & 0x01) << 7;
    CLR_NZC; ccr |= (accA & 0x01);
    r |= accA >> 1; SET_NZ8(r);
    accA = r;
}
void Cpu::rorb() {
    BYTE r = (ccr & 0x01) << 7;
    CLR_NZC; ccr |= (accB & 0x01);
    r |= accB >> 1; SET_NZ8(r);
    accB = r;
}
void Cpu::ror_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    BYTE r = (ccr & 0x01) << 7;
    CLR_NZC; ccr |= (t & 0x01);
    r |= t >> 1; SET_NZ8(r);
    memory->write(accX.w+operand, r);
}
void Cpu::ror_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    BYTE r = (ccr & 0x01) << 7;
    CLR_NZC; ccr |= (t & 0x01);
    r |= t >> 1; SET_NZ8(r);
    memory->write(operand.w, r);
}

/*** Rotate Left Through Carrier ***/
void Cpu::rola() {
    UINT16 t, r;
    t = accA; r = ccr & 0x01; r |= t << 1;
    CLR_NZVC; SET_FLAGS8(t, t, r);
    accA = (BYTE)r;
}
void Cpu::rolb() {
    UINT16 t, r;
    t = accB; r = ccr & 0x01; r |= t << 1;
    CLR_NZVC; SET_FLAGS8(t, t, r);
    accB = (BYTE)r;
}
void Cpu::rol_ix(BYTE operand) {
    UINT16 t, r;
    t = memory->read(operand);
    r = ccr & 0x01; r |= t << 1;
    CLR_NZVC; SET_FLAGS8(t, t, r);
    memory->write(operand, (BYTE)r);
}
void Cpu::rol_ex(WORD_16 operand) {
    UINT16 t, r;
    t = memory->read(operand.w);
    r = ccr & 0x01; r |= t << 1;
    CLR_NZVC; SET_FLAGS8(t, t, r);
    memory->write(operand.w, (BYTE)r);
}

/*** Arithmetic Shift Left ***/
void Cpu::asld() {
    UINT32 r = accD.w << 1;
    CLR_NZVC; SET_FLAGS16(accD.w, accD.w, r);
    accD.w = r;
}
void Cpu::asla() {
    UINT16 r = accA << 1;
    CLR_NZVC; SET_FLAGS8(accA, accA, r);
    accA = (BYTE)r;
}
void Cpu::aslb() {
    UINT16 r = accB << 1;
    CLR_NZVC; SET_FLAGS8(accB, accB, r);
    accB = (BYTE)r;
}
void Cpu::asl_ix(BYTE operand) {
    UINT16 t, r;
    t = memory->read(accX.w+operand);
    r = t << 1;
    CLR_NZVC; SET_FLAGS8(t, t, r);
    memory->write(accX.w+operand, (BYTE)r);
}
void Cpu::asl_ex(WORD_16 operand) {
    UINT16 t, r;
    t = memory->read(operand.w);
    r = t << 1;
    CLR_NZVC; SET_FLAGS8(t, t, r);
    memory->write(operand.w, (BYTE)r);
}

/*** Arithmetic Shift Right ***/
void Cpu::asra() {
    CLR_NZC; ccr |= (accA & 0x01);
    accA >>= 1; accA |= ((accA & 0x40) << 1);
    SET_NZ8(accA);
}
void Cpu::asrb() {
    CLR_NZC; ccr |= (accB & 0x01);
    accB >>= 1; accB |= ((accB & 0x40) << 1);
    SET_NZ8(accB);
}
void Cpu::asr_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    CLR_NZC; ccr |= (t & 0x01);
    t >>= 1; t |= ((t & 0x40) << 1);
    SET_NZ8(t);
    memory->write(accX.w+operand,t);
}
void Cpu::asr_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    CLR_NZC; ccr |= (t & 0x01);
    t >>= 1; t |= ((t & 0x40) << 1);
    SET_NZ8(t);
    memory->write(operand.w,t);
}

/*** Increase ***/
void Cpu::inx() {
    accX.w++;
    CLR_Z; SET_Z16(accX.w);
}
void Cpu::ins() { stackAddr++; }
void Cpu::inca() {
    accA++;
    CLR_NZV; SET_FLAGS8I(accA);
}
void Cpu::incb() {
    accB++;
    CLR_NZV; SET_FLAGS8I(accB);
}
void Cpu::inc_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    t++;
    CLR_NZV; SET_FLAGS8I(t);
    memory->write(accX.w+operand, t);
}
void Cpu::inc_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    t++;
    CLR_NZV; SET_FLAGS8I(t);
    memory->write(operand.w, t);
}

/*** Decrease ***/
void Cpu::dex() {
    accX.w--;
    CLR_Z; SET_Z16(accX.w);
}
void Cpu::des() { stackAddr--; }
void Cpu::deca() {
    accA--;
    CLR_NZV; SET_FLAGS8D(accA);
}
void Cpu::decb() {
    accB--;
    CLR_NZV; SET_FLAGS8D(accB);
}
void Cpu::dec_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    t--;
    CLR_NZV; SET_FLAGS8D(t);
    memory->write(accX.w+operand, t);
}
void Cpu::dec_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    t--;
    CLR_NZV; SET_FLAGS8D(t);
    memory->write(operand.w, t);
}

/*** Subtract ***/
void Cpu::sba() {
    UINT16 t = accA - accB;
    CLR_NZVC; SET_FLAGS8(accA, accB, t);
    accA = (BYTE)t;
}
void Cpu::suba_im(BYTE operand) {
    UINT16 r = accA - operand;
    CLR_NZVC; SET_FLAGS8(accA, operand, r);
    accA = (BYTE)r;
}
void Cpu::suba_di(BYTE operand) {
    BYTE t = memory->read(operand);
    UINT16 r = accA - t;
    CLR_NZVC; SET_FLAGS8(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::suba_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = accA - t;
    CLR_NZVC; SET_FLAGS8(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::suba_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = accA - t;
    CLR_NZVC; SET_FLAGS8(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::subb_im(BYTE operand) {
    UINT16 r = accB - operand;
    CLR_NZVC; SET_FLAGS8(accB, operand, r);
    accB = (BYTE)r;
}
void Cpu::subb_di(BYTE operand) {
    BYTE t = memory->read(operand);
    UINT16 r = accB - t;
    CLR_NZVC; SET_FLAGS8(accB, t, r);
    accB = (BYTE)r;
}
void Cpu::subb_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = accB - t;
    CLR_NZVC; SET_FLAGS8(accB, t, r);
    accB = (BYTE)r;
}
void Cpu::subb_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = accB - t;
    CLR_NZVC; SET_FLAGS8(accB, t, r);
    accB = (BYTE)r;
}
void Cpu::subd_im(WORD_16 operand) {
    UINT32 r = accD.w - operand.w;
    CLR_NZVC; SET_FLAGS16(accD.w, operand.w, r);
    accD.w = r;
}
void Cpu::subd_di(BYTE operand) {
    WORD_16 t = memory->read16(operand);
    UINT32 r = accD.w - t.w;
    CLR_NZVC; SET_FLAGS16(accD.w, t.w, r);
    accD.w = r;
}
void Cpu::subd_ix(BYTE operand) {
    WORD_16 t = memory->read16(accX.w+operand);
    UINT32 r = accD.w - t.w;
    CLR_NZVC; SET_FLAGS16(accD.w, t.w, r);
    accD.w = r;
}
void Cpu::subd_ex(WORD_16 operand) {
    WORD_16 t = memory->read16(operand.w);
    UINT32 r = accD.w - t.w;
    CLR_NZVC; SET_FLAGS16(accD.w, t.w, r);
    accD.w = r;
}

/*** Subtract with Carrier ***/
void Cpu::sbca_im(BYTE operand) {
    UINT16 r = accA - operand - (ccr & 0x01);
    CLR_NZVC; SET_FLAGS8(accA, operand, r);
    accA = (BYTE)r;
}
void Cpu::sbca_di(BYTE operand) {
    BYTE t = memory->read(operand);
    UINT16 r = accA - t - (ccr & 0x01);
    CLR_NZVC; SET_FLAGS8(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::sbca_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = accA - t - (ccr & 0x01);
    CLR_NZVC; SET_FLAGS8(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::sbca_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = accA - t - (ccr & 0x01);
    CLR_NZVC; SET_FLAGS8(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::sbcb_im(BYTE operand) {
    UINT16 r = accB - operand - (ccr & 0x01);
    CLR_NZVC; SET_FLAGS8(accB, operand, r);
    accB = (BYTE)r;
}
void Cpu::sbcb_di(BYTE operand) {
    BYTE t = memory->read(operand);
    UINT16 r = accB - t - (ccr & 0x01);
    CLR_NZVC; SET_FLAGS8(accB, t, r);
    accB = (BYTE)r;
}
void Cpu::sbcb_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = accB - t - (ccr & 0x01);
    CLR_NZVC; SET_FLAGS8(accB, t, r);
    accB = (BYTE)r;
}
void Cpu::sbcb_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = accB - t - (ccr & 0x01);
    CLR_NZVC; SET_FLAGS8(accB, t, r);
    accB = (BYTE)r;
}

/*** Compare ***/
void Cpu::cba() {
    UINT16 t = accA - accB;
    CLR_NZVC; SET_FLAGS8(accA, accB, t);
}
void Cpu::tim_ix(WORD_16 operand) {
    BYTE r = memory->read(accX.w+operand.b.l);
    r &= operand.b.h;
    CLR_NZV; SET_NZ8(r);
}
void Cpu::tim_di(WORD_16 operand) {
    BYTE r = memory->read(operand.b.l);
    r &= operand.b.h;
    CLR_NZV; SET_NZ8(r);
}
void Cpu::cmpa_im(BYTE operand) {
    UINT16 r = accA - operand;
    CLR_NZVC; SET_FLAGS8(accA, operand, r);
}
void Cpu::cmpa_di(BYTE operand) {
    BYTE t = memory->read(operand);
    UINT16 r = accA - t;
    CLR_NZVC; SET_FLAGS8(accA, t, r);
}
void Cpu::cmpa_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = accA - t;
    CLR_NZVC; SET_FLAGS8(accA, t, r);
}
void Cpu::cmpa_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = accA - t;
    CLR_NZVC; SET_FLAGS8(accA, t, r);
}
void Cpu::cmpb_im(BYTE operand) {
    UINT16 r = accB - operand;
    CLR_NZVC; SET_FLAGS8(accB, operand, r);
}
void Cpu::cmpb_di(BYTE operand) {
    BYTE t = memory->read(operand);
    UINT16 r = accB - t;
    CLR_NZVC; SET_FLAGS8(accB, t, r);
}
void Cpu::cmpb_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = accB - t;
    CLR_NZVC; SET_FLAGS8(accB, t, r);
}
void Cpu::cmpb_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = accB - t;
    CLR_NZVC; SET_FLAGS8(accB, t, r);
}
void Cpu::cpx_im(WORD_16 operand) {
    UINT32 r = accX.w - operand.w;
    CLR_NZVC; SET_FLAGS16(accX.w, operand.w, r);
}
void Cpu::cpx_di(BYTE operand) {
    WORD_16 t = memory->read16(operand);
    UINT32 r = accX.w - t.w;
    CLR_NZVC; SET_FLAGS16(accX.w, t.w, r);
}
void Cpu::cpx_ix(BYTE operand) {
    WORD_16 t = memory->read16(accX.w+operand);
    UINT32 r = accX.w - t.w;
    CLR_NZVC; SET_FLAGS16(accX.w, t.w, r);
}
void Cpu::cpx_ex(WORD_16 operand) {
    WORD_16 t = memory->read16(operand.w);
    UINT32 r = accX.w - t.w;
    CLR_NZVC; SET_FLAGS16(accX.w, t.w, r);
}

/*** Test or Test Bit ***/
void Cpu::tsta() {
    CLR_NZVC; SET_NZ8(accA);
}
void Cpu::tstb() {
    CLR_NZVC; SET_NZ8(accB);
}
void Cpu::tst_ix(BYTE operand) {
	CLR_NZVC; BYTE val = memory->read(accX.w + operand); SET_NZ8(val);
}
void Cpu::tst_ex(WORD_16 operand) {
	CLR_NZVC; BYTE val = memory->read(operand.w); SET_NZ8(val);
}
void Cpu::bita_im(BYTE operand) {
    BYTE r = accA & operand;
    CLR_NZV; SET_NZ8(r);
}
void Cpu::bita_di(BYTE operand) {
    BYTE r = accA & memory->read(operand);
    CLR_NZV; SET_NZ8(r);
}
void Cpu::bita_ix(BYTE operand) {
    BYTE r = accA & memory->read(accX.w+operand);
    CLR_NZV; SET_NZ8(r);
}
void Cpu::bita_ex(WORD_16 operand) {
    BYTE r = accA & memory->read(operand.w);
    CLR_NZV; SET_NZ8(r);
}
void Cpu::bitb_im(BYTE operand) {
    BYTE r = accB & operand;
    CLR_NZV; SET_NZ8(r);
}
void Cpu::bitb_di(BYTE operand) {
    BYTE r = accB & memory->read(operand);
    CLR_NZV; SET_NZ8(r);
}
void Cpu::bitb_ix(BYTE operand) {
    BYTE r = accB & memory->read(accX.w+operand);
    CLR_NZV; SET_NZ8(r);
}
void Cpu::bitb_ex(WORD_16 operand) {
    BYTE r = accB & memory->read(operand.w);
    CLR_NZV; SET_NZ8(r);
}

/*** Addition ***/
void Cpu::aba() {
    UINT16 t;
    t = accA + accB;
    CLR_HNZVC; SET_FLAGS8(accA, accB, t);
    SET_H(accA, accB, t);
    accA = (BYTE)t;
}
void Cpu::abx() { accX.w += accB; }
void Cpu::adda_im(BYTE operand) {
    UINT16 r = accA + operand;
    CLR_HNZVC; SET_FLAGS8(accA, operand, r); SET_H(accA, operand, r);
    accA = (BYTE)r;
}
void Cpu::adda_di(BYTE operand) {
    BYTE t = memory->read(operand);
    UINT16 r = accA + t;
    CLR_HNZVC; SET_FLAGS8(accA, t, r); SET_H(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::adda_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = accA + t;
    CLR_HNZVC; SET_FLAGS8(accA, t, r); SET_H(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::adda_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = accA + t;
    CLR_HNZVC; SET_FLAGS8(accA, t, r); SET_H(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::addb_im(BYTE operand) {
    UINT16 r = accB + operand;
    CLR_HNZVC; SET_FLAGS8(accB, operand, r); SET_H(accB, operand, r);
    accB = (BYTE)r;
}
void Cpu::addb_di(BYTE operand) {
    BYTE t = memory->read(operand);
    UINT16 r = accB + t;
    CLR_HNZVC; SET_FLAGS8(accB, t, r); SET_H(accB, t, r);
    accB = (BYTE)r;
}
void Cpu::addb_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = accB + t;
    CLR_HNZVC; SET_FLAGS8(accB, t, r); SET_H(accB, t, r);
    accB = (BYTE)r;
}
void Cpu::addb_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = accB + t;
    CLR_HNZVC; SET_FLAGS8(accB, t, r); SET_H(accB, t, r);
    accB = (BYTE)r;
}
void Cpu::addd_im(WORD_16 operand) {
    UINT32 r = accD.w + operand.w;
    CLR_NZVC; SET_FLAGS16(accD.w, operand.w, r);
    accD.w = (UINT16)r;
}
void Cpu::addd_di(BYTE operand) {
    WORD_16 t = memory->read16(operand);
    UINT32 r = accD.w + t.w;
    CLR_NZVC; SET_FLAGS16(accD.w, t.w, r);
    accD.w = r;
}
void Cpu::addd_ix(BYTE operand) {
    WORD_16 t = memory->read16(accX.w+operand);
    UINT32 r = accD.w + t.w;
    CLR_NZVC; SET_FLAGS16(accD.w, t.w, r);
    accD.w = r;
}
void Cpu::addd_ex(WORD_16 operand) {
    WORD_16 t = memory->read16(operand.w);
    UINT32 r = accD.w + t.w;
    CLR_NZVC; SET_FLAGS16(accD.w, t.w, r);
    accD.w = r;
}

/*** Addition with Carrier **/
void Cpu::adca_im(BYTE operand) {
    UINT16 r = accA + operand + (ccr & 0x01);
    CLR_HNZVC; SET_FLAGS8(accA, operand, r); SET_H(accA, operand, r);
    accA = (BYTE)r;
}
void Cpu::adca_di(BYTE operand) {
    BYTE t = memory->read(operand);
    UINT16 r = accA + t + (ccr & 0x01);
    CLR_HNZVC; SET_FLAGS8(accA, t, r); SET_H(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::adca_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = accA + t + (ccr & 0x01);
    CLR_HNZVC; SET_FLAGS8(accA, t, r); SET_H(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::adca_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = accA + t + (ccr & 0x01);
    CLR_HNZVC; SET_FLAGS8(accA, t, r); SET_H(accA, t, r);
    accA = (BYTE)r;
}
void Cpu::adcb_im(BYTE operand) {
    UINT16 r = accB + operand + (ccr & 0x01);
    CLR_HNZVC; SET_FLAGS8(accB, operand, r); SET_H(accB, operand, r);
    accB = (BYTE)r;
}
void Cpu::adcb_di(BYTE operand) {
    BYTE t = memory->read(operand);
    UINT16 r = accB + t + (ccr & 0x01);
    CLR_HNZVC; SET_FLAGS8(accB, t, r); SET_H(accB, t, r);
    accB = (BYTE)r;
}
void Cpu::adcb_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = accB + t + (ccr & 0x01);
    CLR_HNZVC; SET_FLAGS8(accB, t, r); SET_H(accB, t, r);
    accB = (BYTE)r;
}
void Cpu::adcb_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = accB + t + (ccr & 0x01);
    CLR_HNZVC; SET_FLAGS8(accB, t, r); SET_H(accB, t, r);
    accB = (BYTE)r;
}

/*** Negative ***/
void Cpu::nega() {
    UINT16 r = -accA;
    CLR_NZVC; SET_FLAGS8(0, accA, r);
    accA = (BYTE)r;
}
void Cpu::negb() {
    UINT16 r = -accB;
    CLR_NZVC; SET_FLAGS8(0, accB, r);
    accB = (BYTE)r;
}
void Cpu::neg_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    UINT16 r = -t;
    CLR_NZVC; SET_FLAGS8(0, t, r);
    memory->write(accX.w+operand, (BYTE)r);
}
void Cpu::neg_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    UINT16 r = -t;
    CLR_NZVC; SET_FLAGS8(0, t, r);
    memory->write(operand.w, (BYTE)r);
}

/*** Complement ***/
void Cpu::coma() {
    accA = ~accA;
    CLR_NZV; SET_NZ8(accA); SEC;
}
void Cpu::comb() {
    accB = ~accB;
    CLR_NZV; SET_NZ8(accB); SEC;
}
void Cpu::com_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    t = ~t;
    CLR_NZV; SET_NZ8(t); SEC;
    memory->write(accX.w+operand, t);
}
void Cpu::com_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    t = ~t;
    CLR_NZV; SET_NZ8(t); SEC;
    memory->write(operand.w, t);
}

/*** Clear ***/
void Cpu::clra() {
    accA = 0;
    CLR_NZVC; SEZ;
}
void Cpu::clrb() {
    accB = 0;
    CLR_NZVC; SEZ;
}
void Cpu::clr_ix(BYTE operand) {
    memory->write(accX.w+operand, 0);
    CLR_NZVC; SEZ;
}
void Cpu::clr_ex(WORD_16 operand) {
    memory->write(operand.w, 0);
    CLR_NZVC; SEZ;
}

/*** Logical And ***/
void Cpu::aim_ix(WORD_16 operand) {
    BYTE r = memory->read(accX.w+operand.b.l);
    r &= operand.b.h;
    CLR_NZV; SET_NZ8(r);
    memory->write(accX.w+operand.b.l, r);
}
void Cpu::aim_di(WORD_16 operand) {
    BYTE r = memory->read(operand.b.l);
    r &= operand.b.h;
    CLR_NZV; SET_NZ8(r);
    memory->write(operand.b.l, r);
}
void Cpu::anda_im(BYTE operand) {
    accA &= operand;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::anda_di(BYTE operand) {
    BYTE t = memory->read(operand);
    accA &= t;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::anda_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    accA &= t;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::anda_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    accA &= t;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::andb_im(BYTE operand) {
    accB &= operand;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::andb_di(BYTE operand) {
    BYTE t = memory->read(operand);
    accB &= t;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::andb_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    accB &= t;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::andb_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    accB &= t;
    CLR_NZV; SET_NZ8(accB);
}

/*** Logical Or ***/
void Cpu::oim_ix(WORD_16 operand) {
    BYTE r = memory->read(accX.w+operand.b.l);
    r |= operand.b.h;
    CLR_NZV; SET_NZ8(r);
    memory->write(accX.w+operand.b.l, r);
}
void Cpu::oim_di(WORD_16 operand) {
    BYTE r = memory->read(operand.b.l);
    r |= operand.b.h;
    CLR_NZV; SET_NZ8(r);
    memory->write(operand.b.l, r);
}
void Cpu::ora_im(BYTE operand) {
    accA |= operand;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::ora_di(BYTE operand) {
    BYTE t = memory->read(operand);
    accA |= t;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::ora_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    accA |= t;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::ora_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    accA |= t;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::orb_im(BYTE operand) {
    accB |= operand;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::orb_di(BYTE operand) {
    BYTE t = memory->read(operand);
    accB |= t;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::orb_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    accB |= t;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::orb_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    accB |= t;
    CLR_NZV; SET_NZ8(accB);
}

/*** Exponential Or ***/
void Cpu::eim_ix(WORD_16 operand) {
    BYTE r = memory->read(accX.w+operand.b.l);
    r ^= operand.b.h;
    CLR_NZV; SET_NZ8(r);
    memory->write(accX.w+operand.b.l, r);
}
void Cpu::eim_di(WORD_16 operand) {
    BYTE r = memory->read(operand.b.l);
    r ^= operand.b.h;
    CLR_NZV; SET_NZ8(r);
    memory->write(operand.b.l, r);
}
void Cpu::eora_im(BYTE operand) {
    accA ^= operand;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::eora_di(BYTE operand) {
    BYTE t = memory->read(operand);
    accA ^= t;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::eora_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    accA ^= t;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::eora_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    accA ^= t;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::eorb_im(BYTE operand) {
    accB ^= operand;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::eorb_di(BYTE operand) {
    BYTE t = memory->read(operand);
    accB ^= t;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::eorb_ix(BYTE operand) {
    BYTE t = memory->read(accX.w+operand);
    accB ^= t;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::eorb_ex(WORD_16 operand) {
    BYTE t = memory->read(operand.w);
    accB ^= t;
    CLR_NZV; SET_NZ8(accB);
}

/*** Jump ***/
void Cpu::jmp_ix(BYTE operand) {
    pc = accX.w + operand;
}
void Cpu::jmp_ex(WORD_16 operand) {
    pc = operand.w;
}
void Cpu::jsr_di(BYTE operand) {
    pushAddr(pc);
    pc = operand;
}
void Cpu::jsr_ix(BYTE operand) {
    pushAddr(pc);
    pc = accX.w + operand;
}
void Cpu::jsr_ex(WORD_16 operand) {
    pushAddr(pc);
    pc = operand.w;
}

/*** Load ***/
void Cpu::lda_im(BYTE operand) {
    accA = operand;
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::lda_di(BYTE operand) {
    accA = memory->read(operand);
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::lda_ix(BYTE operand) {
    accA = memory->read(accX.w+operand);
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::lda_ex(WORD_16 operand) {
    accA = memory->read(operand.w);
    CLR_NZV; SET_NZ8(accA);
}
void Cpu::ldb_im(BYTE operand) {
    accB = operand;
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::ldb_di(BYTE operand) {
    accB = memory->read(operand);
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::ldb_ix(BYTE operand) {
    accB = memory->read(accX.w+operand);
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::ldb_ex(WORD_16 operand) {
    accB = memory->read(operand.w);
    CLR_NZV; SET_NZ8(accB);
}
void Cpu::ldd_im(WORD_16 operand) {
    accD = operand;
    CLR_NZV; SET_NZ16(accD.w);
}
void Cpu::ldd_di(BYTE operand) {
    accD = memory->read16(operand);
    CLR_NZV; SET_NZ16(accD.w);
}
void Cpu::ldd_ix(BYTE operand) {
    accD = memory->read16(accX.w+operand);
    CLR_NZV; SET_NZ16(accD.w);
}
void Cpu::ldd_ex(WORD_16 operand) {
    accD = memory->read16(operand.w);
    CLR_NZV; SET_NZ16(accD.w);
}
void Cpu::ldx_im(WORD_16 operand) {
    accX = operand;
    CLR_NZV; SET_NZ16(accX.w);
}
void Cpu::ldx_di(BYTE operand) {
    accX = memory->read16(operand);
    CLR_NZV; SET_NZ16(accX.w);
}
void Cpu::ldx_ix(BYTE operand) {
    accX = memory->read16(accX.w+operand);
    CLR_NZV; SET_NZ16(accX.w);
}
void Cpu::ldx_ex(WORD_16 operand) {
    accX = memory->read16(operand.w);
    CLR_NZV; SET_NZ16(accX.w);
}
void Cpu::lds_im(WORD_16 operand) {
    stackAddr = operand.w;
    CLR_NZV; SET_NZ16(stackAddr);
}
void Cpu::lds_di(BYTE operand) {
    stackAddr = memory->readAddress(operand);
    CLR_NZV; SET_NZ16(stackAddr);
}
void Cpu::lds_ix(BYTE operand) {
    stackAddr = memory->readAddress(accX.w+operand);
    CLR_NZV; SET_NZ16(stackAddr);
}
void Cpu::lds_ex(WORD_16 operand) {
    stackAddr = memory->readAddress(operand.w);
    CLR_NZV; SET_NZ16(stackAddr);
}

/*** Save ***/
void Cpu::sta_di(BYTE operand) {
    CLR_NZV; SET_NZ8(accA);
    memory->write(operand, accA);
}
void Cpu::sta_ix(BYTE operand) {
    CLR_NZV; SET_NZ8(accA);
    memory->write(accX.w+operand, accA);
}
void Cpu::sta_ex(WORD_16 operand) {
    CLR_NZV; SET_NZ8(accA);
    memory->write(operand.w, accA);
}
void Cpu::stb_di(BYTE operand) {
    CLR_NZV; SET_NZ8(accB);
    memory->write(operand, accB);
}
void Cpu::stb_ix(BYTE operand) {
    CLR_NZV; SET_NZ8(accB);
    memory->write(accX.w+operand, accB);
}
void Cpu::stb_ex(WORD_16 operand) {
    CLR_NZV; SET_NZ8(accB);
    memory->write(operand.w, accB);
}
void Cpu::std_di(BYTE operand) {
    CLR_NZV; SET_NZ16(accD.w);
    memory->write16(operand, accD);
}
void Cpu::std_ix(BYTE operand) {
    CLR_NZV; SET_NZ16(accD.w);
    memory->write16(accX.w+operand, accD);
}
void Cpu::std_ex(WORD_16 operand) {
    CLR_NZV; SET_NZ16(accD.w);
    memory->write16(operand.w, accD);
}
void Cpu::stx_di(BYTE operand) {
    CLR_NZV; SET_NZ16(accX.w);
    memory->write16(operand, accX);
}
void Cpu::stx_ix(BYTE operand) {
    CLR_NZV; SET_NZ16(accX.w);
    memory->write16(accX.w+operand, accX);
}
void Cpu::stx_ex(WORD_16 operand) {
    CLR_NZV; SET_NZ16(accX.w);
    memory->write16(operand.w, accX);
}
void Cpu::sts_di(BYTE operand) {
    CLR_NZV; SET_NZ16(stackAddr);
    memory->writeAddress(operand, stackAddr);
}
void Cpu::sts_ix(BYTE operand) {
    CLR_NZV; SET_NZ16(stackAddr);
    memory->writeAddress(accX.w+operand, stackAddr);
}
void Cpu::sts_ex(WORD_16 operand) {
    CLR_NZV; SET_NZ16(stackAddr);
    memory->writeAddress(operand.w, stackAddr);
}
