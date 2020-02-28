// NOTE: This does not really test copying from storage to ABI directly,
    // because it will always copy to memory first.
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
// test() -> 0x20, 0x8, -0x1, -0x1, 0x8, -0x10, -0x2, 0x6, 0x8, -0x1

