
		contract C {
			function f(uint a) public returns (uint b) {
				assembly {
					if gt(a, 1) { b := 2 }
				}
			}
		}
	
// ====
// compileViaYul: also
// ----
// f(uint256): 0 -> 0
// f(uint256): 1 -> 0
// f(uint256): 2 -> 2
// f(uint256): 3 -> 2

