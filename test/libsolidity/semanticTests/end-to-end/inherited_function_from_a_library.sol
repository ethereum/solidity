
		library A { function f() internal returns (uint) { return 1; } }
		contract B {
			function f() internal returns (uint) { return 2; }
			function g() public returns (uint) {
				return A.f();
			}
		}
	
// ----
// g() -> 1

