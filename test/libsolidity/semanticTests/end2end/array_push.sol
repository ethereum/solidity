
		contract c {
			uint[] data;
			function test() public returns (uint x, uint y, uint z, uint l) {
				data.push(5);
				x = data[0];
				data.push(4);
				y = data[1];
				data.push(3);
				l = data.length;
				z = data[2];
			}
		}
	
// ----
// test() -> 0x5, 0x4, 0x3, 0x3

