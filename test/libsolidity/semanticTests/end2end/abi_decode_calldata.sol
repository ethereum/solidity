
		contract C {
			function f(bytes calldata data) external pure returns (uint, bytes memory r) {
				return abi.decode(data, (uint, bytes));
			}
		}
	
// ----
// f(bytes): 0x20, 0x80, 0x21, 0x40, 0x7, "abcdefg" -> 0x21, 0x40, 0x7, "abcdefg"

