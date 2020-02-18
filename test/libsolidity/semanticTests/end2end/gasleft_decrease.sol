
		contract C {
			uint v;
			function f() public returns (bool) {
				uint startGas = gasleft();
				v++;
				assert(startGas > gasleft());
				return true;
			}
			function g() public returns (bool) {
				uint startGas = gasleft();
				assert(startGas > gasleft());
				return true;
			}
		}
	
// ----
// f() -> 0x1
// g() -> 0x1

