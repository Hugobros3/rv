#include "rv/Mask.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"

using namespace llvm;

namespace rv {

// struct Mask
void Mask::print(llvm::raw_ostream &out) const {
  out << "Mask {";
  bool hasText = false;
  if (Predicate) {
    out << "P: ";
    Predicate->printAsOperand(out);
    hasText = true;
  }
  if (ActiveVectorLength) {
    if (hasText)
      out << ", ";
    ActiveVectorLength->printAsOperand(out);
  }
  out << "}";
}

void Mask::dump() const { print(errs()); }

Mask Mask::inferFromPredicate(llvm::Value &Pred) {
  auto ConstPred = dyn_cast<Constant>(&Pred);

  // Normalize to cannonical all-true mask
  if (ConstPred && ConstPred->isAllOnesValue()) {
    return Mask::getAllTrue();
  }

  // TODO infer AVL
  return Mask(&Pred, nullptr);
}

Mask Mask::fromVectorLength(llvm::Value &EVLen) {
  return Mask(nullptr, &EVLen);
}

bool Mask::operator==(const Mask &B) const {
  return (Predicate == B.Predicate) &&
         (ActiveVectorLength == B.ActiveVectorLength);
}

Mask Mask::getAllFalse(LLVMContext &Ctx) {
  auto VLZero = ConstantInt::getNullValue(Type::getInt32Ty(Ctx));
  return Mask::fromVectorLength(*VLZero);
}

llvm::Value &Mask::requestPredAsValue(llvm::LLVMContext &Ctx) const {
  if (getPred())
    return *getPred();
  return *ConstantInt::getTrue(Ctx);
}

llvm::Value &Mask::requestAVLAsValue(llvm::LLVMContext &Ctx) const {
  if (getAVL())
    return *getAVL();
  return *ConstantInt::get(Type::getInt32Ty(Ctx), -1, true);
}

bool Mask::knownAllTrue() const { return (!getPred() && !getAVL()); }

bool Mask::knownAllFalse() const {
  // AVL == 0
  if (getAVL()) {
    auto ConstAVL = dyn_cast<ConstantInt>(getAVL());
    if (ConstAVL && ConstAVL->isNullValue())
      return true;
  }

  // Pred == 0
  if (getPred()) {
    auto ConstPred = dyn_cast<Constant>(getAVL());
    if (ConstPred && ConstPred->isNullValue())
      return true;
  }

  // Don't know
  return false;
}

} // namespace rv

namespace llvm {
raw_ostream &operator<<(raw_ostream &Out, const rv::Mask &M) {
  M.print(Out);
  return Out;
}
} // namespace llvm