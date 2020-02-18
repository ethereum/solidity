
		contract C {
			function f() public returns (uint r) {
				uint; uint; uint; uint;
				int x = -7;
				return uint(x);
			}
		}
	
// ====
// optimize-yul: false
// ----
// f() -> 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff9

