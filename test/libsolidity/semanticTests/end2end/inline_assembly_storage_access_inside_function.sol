
		contract C {
			uint16 x;
			uint16 public y;
			uint public z;
			function f() public returns (bool) {
				uint off1;
				uint off2;
				assembly {
					function f() -> o1 {
						sstore(z_slot, 7)
						o1 := y_offset
					}
					off2 := f()
				}
				assert(off2 == 2);
				return true;
			}
		}
	
// ----
// f() -> 0x1
// z() -> 0x07

