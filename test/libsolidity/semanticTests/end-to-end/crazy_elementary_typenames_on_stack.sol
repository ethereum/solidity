
		contract C {
			function f() public returns (uint r) {
				uint; uint; uint; uint;
				int x = -7;
				return uint(x);
			}
		}
	
// ====
// compileViaYul: also
// ----
// f() -> -7

