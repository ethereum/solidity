
		contract A {
			uint constant x = 7;
		}
		contract B is A {
			function f() public returns (uint) {
				return A.x;
			}
		}
	
// ----
// f() -> 7

