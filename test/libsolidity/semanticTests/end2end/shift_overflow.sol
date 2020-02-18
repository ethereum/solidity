
		contract C {
			function leftU(uint8 x, uint8 y) public returns (uint8) {
				return x << y;
			}
			function leftS(int8 x, int8 y) public returns (int8) {
				return x << y;
			}
		}
	
// ----
// leftU(uint8,uint8): 0xff, 0x8 -> 0x00
// leftU(uint8,uint8): 0xff, 0x1 -> 0xfe
// leftU(uint8,uint8): 0xff, 0x0 -> 0xff
// leftS(int8,int8): 0x1, 0x7 -> 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff80
// leftS(int8,int8): 0x1, 0x6 -> 0x40

