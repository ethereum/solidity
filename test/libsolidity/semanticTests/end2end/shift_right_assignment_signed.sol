
			contract C {
				function f(int a, int b) public returns (int) {
					a >>= b;
					return a;
				}
			}
		
// ----
// f(int256,int256): 0x4266, 0x00 -> 0x4266
// f(int256,int256): 0x4266, 0x08 -> 0x42
// f(int256,int256): 0x4266, 0x10 -> 0x00
// f(int256,int256): 0x4266, 0x11 -> 0x00

