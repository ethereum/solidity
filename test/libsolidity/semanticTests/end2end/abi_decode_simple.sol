
		contract C {
			function f(bytes memory data) public pure returns (uint, bytes memory) {
				return abi.decode(data, (uint, bytes));
			}
		}
	
// ----
// f(bytes): 0x20, 0x80, 0x21, 0x40, 0x7, "abcdefg" -> 0x21, 0x40, 0x7, "abcdefg"

