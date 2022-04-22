from typing import Any
from z3 import BitVecVal, BV2Int, If, LShR, UDiv, ULT, UGT, URem

def ADD(x: Any, y: Any) -> Any:
	return x + y

def MUL(x: Any, y: Any) -> Any:
	return x * y

def SUB(x: Any, y: Any) -> Any:
	return x - y

def DIV(x: Any, y: Any) -> Any:
	return If(y == 0, 0, UDiv(x, y))

def SDIV(x: Any, y: Any) -> Any:
	return If(y == 0, 0, x / y)

def MOD(x: Any, y: Any) -> Any:
	return If(y == 0, 0, URem(x, y))

def SMOD(x: Any, y: Any) -> Any:
	return If(y == 0, 0, x % y)

def LT(x: Any, y: Any) -> Any:
	return If(ULT(x, y), BitVecVal(1, x.size()), BitVecVal(0, x.size()))

def GT(x: Any, y: Any) -> Any:
	return If(UGT(x, y), BitVecVal(1, x.size()), BitVecVal(0, x.size()))

def SLT(x: Any, y: Any) -> Any:
	return If(x < y, BitVecVal(1, x.size()), BitVecVal(0, x.size()))

def SGT(x: Any, y: Any) -> Any:
	return If(x > y, BitVecVal(1, x.size()), BitVecVal(0, x.size()))

def EQ(x: Any, y: Any) -> Any:
	return If(x == y, BitVecVal(1, x.size()), BitVecVal(0, x.size()))

def ISZERO(x: Any) -> Any:
	return If(x == 0, BitVecVal(1, x.size()), BitVecVal(0, x.size()))

def AND(x: Any, y: Any) -> Any:
	return x & y

def OR(x: Any, y: Any) -> Any:
	return x | y

def NOT(x: Any) -> Any:
	return ~(x)

def SHL(x: Any, y: Any) -> Any:
	return y << x

def SHR(x: Any, y: Any) -> Any:
	return LShR(y, x)

def SAR(x: Any, y: Any) -> Any:
	return y >> x

def BYTE(i: Any, x: Any) -> Any:
	bit = (i + 1) * 8
	return If(
		UGT(i, x.size() / 8 - 1),
		BitVecVal(0, x.size()),
		(LShR(x, (x.size() - bit))) & 0xff
	)

def SIGNEXTEND(i: Any, x: Any) -> Any:
	bitBV = i * 8 + 7
	bitInt = BV2Int(i) * 8 + 7
	test = BitVecVal(1, x.size()) << bitBV
	mask = test - 1
	return If(
		bitInt >= x.size(),
		x,
		If(
			(x & test) == 0,
			x & mask,
			x | ~mask
		)
	)
