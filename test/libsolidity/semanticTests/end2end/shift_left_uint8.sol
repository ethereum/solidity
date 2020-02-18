
		contract C {
			function f(uint8 a, uint8 b) public returns (uint) {
				return a << b;
			}
		}
	
// ----
// f(uint8,uint8): 0x66, 0x00 -> 0x66
// f(uint8,uint8): 0x66, 0x08 -> 0x00

