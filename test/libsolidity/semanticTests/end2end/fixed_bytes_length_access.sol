
		contract C {
			byte a;
			function f(bytes32 x) public returns (uint, uint, uint) {
				return (x.length, bytes16(uint128(2)).length, a.length + 7);
			}
		}
	
// ----
// f(bytes32): "789" -> 0x20, 0x10, 0x08

