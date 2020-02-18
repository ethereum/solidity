
		contract C {
			modifier m {
				uint a = 1;
				assembly {
					a := 2
				}
				if (a != 2)
					revert();
				_;
			}
			function f() m public returns (bool) {
				return true;
			}
		}
	
// ====
// optimize-yul: false
// ----
// f() -> 0x1

