
		contract C {
			uint constant LEN = 3;
			uint[LEN] public a;

			constructor(uint[LEN] memory _a) public {
				a = _a;
			}
		}
	
// ----
// a(uint256): 0 -> 1
// a(uint256): 1 -> 2
// a(uint256): 2 -> 3

