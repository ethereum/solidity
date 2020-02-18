
		contract C {
			function f(uint i) public returns (string memory) {
				string[4] memory x = ["This", "is", "an", "array"];
				return (x[i]);
			}
		}
	
// ----
// f(uint256): 0x00 -> 0x20, 0x04, "This"
// f(uint256): 0x01 -> 0x20, 0x02, "is"
// f(uint256): 0x02 -> 0x20, 0x02, "an"
// f(uint256): 0x03 -> 0x20, 0x05, "array"

