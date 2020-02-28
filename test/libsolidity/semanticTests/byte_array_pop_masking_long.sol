
		contract c {
			bytes data;
			function test() public returns (bytes memory) {
				for (uint i = 0; i < 34; i++)
					data.push(0x03);
				data.pop();
				return data;
			}
		}
	
// ----
// test() -> 0x20, 0x21, 0x303030303030303030303030303030303030303030303030303030303030303, 0x3

