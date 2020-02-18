
		contract c {
			int16[] x;
			function test() public returns (int16[] memory) {
				x.push(int16(-1));
				x.push(int16(-1));
				x.push(int16(8));
				x.push(int16(-16));
				x.push(int16(-2));
				x.push(int16(6));
				x.push(int16(8));
				x.push(int16(-1));
				return x;
			}
		}
	
// ----
// test() -> 0x20, 0x8, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 0x08, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff0, 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 0x06, 0x08, 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff

