
		contract C {
			function f(uint32 a, uint32 b) public returns (uint) {
				return a >> b;
			}
		}
	
// ----
// f(uint32,uint32): 0x4266, 0x00 -> 0x4266
// f(uint32,uint32): 0x4266, 0x08 -> 0x42
// f(uint32,uint32): 0x4266, 0x10 -> 0x00
// f(uint32,uint32): 0x4266, 0x11 -> 0x00

