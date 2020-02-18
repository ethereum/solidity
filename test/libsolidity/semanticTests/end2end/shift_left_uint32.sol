
		contract C {
			function f(uint32 a, uint32 b) public returns (uint) {
				return a << b;
			}
		}
	
// ----
// f(uint32,uint32): 0x4266, 0x00 -> 0x4266
// f(uint32,uint32): 0x4266, 0x08 -> 0x426600
// f(uint32,uint32): 0x4266, 0x10 -> 0x42660000
// f(uint32,uint32): 0x4266, 0x11 -> 0x84cc0000
// f(uint32,uint32): 0x4266, 0x20 -> 0x00

