#include <cmath>

template <typename FPType>
FPInstruction<FPType>::FPInstruction(FPType& ft) : ft(ft) {}

template <typename FPType>
FRInstruction<FPType>::FRInstruction(FR_INSTR_ARGS) : FPInstruction<FPType>(ft), fs(fs), fd(fd) {}

#define FR_INSTR_INIT(x) template <typename FPType> x<FPType>::x(FR_INSTR_ARGS) : FRInstruction<FPType>(ft, fs, fd) {} template <typename FPType> void x<FPType>::run()
FR_INSTR_INIT(FPAdd) { this->fd = this->fs + this->ft; }
FR_INSTR_INIT(FPDivide) { this->fd = this->fs / this->ft; }
FR_INSTR_INIT(FPMultiply) { this->fd = this->fs * this->ft; }
FR_INSTR_INIT(FPSubtract) { this->fd = this->fs - this->ft; }


