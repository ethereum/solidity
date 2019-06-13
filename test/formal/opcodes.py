from z3 import *

def ADD(x, y):
	return x + y

def MUL(x, y):
	return x * y

def SUB(x, y):
	return x - y

def DIV(x, y):
	return If(y == 0, 0, UDiv(x, y))

def SDIV(x, y):
	return If(y == 0, 0, x / y)

def MOD(x, y):
	return If(y == 0, 0, URem(x, y))

def SMOD(x, y):
	return If(y == 0, 0, x % y)

def LT(x, y):
	return ULT(x, y)

def GT(x, y):
	return UGT(x, y)

def SLT(x, y):
	return x < y

def SGT(x, y):
	return x > y

def AND(x, y):
	return x & y

def SHL(x, y):
	return y << x

def SHR(x, y):
	return LShR(y, x)

def SAR(x, y):
	return y >> x
