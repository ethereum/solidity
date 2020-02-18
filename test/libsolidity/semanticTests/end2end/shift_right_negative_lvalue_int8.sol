
			contract C {
				function f(int8 a, int8 b) public returns (int) {
					return a >> b;
				}
			}
		
// ----
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbe, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbe
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbe, 0x01 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffdf
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbe, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffb
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbe, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbe, 0x10 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbe, 0x11 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbd, 0x00 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbd
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbd, 0x01 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffde
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbd, 0x04 -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffb
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbd, 0x08 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbd, 0x10 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
// f(int8,int8): 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffbd, 0x11 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff

