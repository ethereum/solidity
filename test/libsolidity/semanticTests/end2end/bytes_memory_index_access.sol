
		contract Test {
			function set(bytes memory _data, uint i) public returns (uint l, byte c) {
				l = _data.length;
				c = _data[i];
			}
		}
	
// ====
// optimize-yul: false
// ----
// set(bytes,uint256): 0x40, 0x03, 0x08, "abcdefgh" -> 0x08, "d"

