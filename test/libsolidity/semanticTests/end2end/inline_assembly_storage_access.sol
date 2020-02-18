
		contract C {
			uint16 x;
			uint16 public y;
			uint public z;
			function f() public returns (bool) {
				uint off1;
				uint off2;
				assembly {
					sstore(z_slot, 7)
					off1 := z_offset
					off2 := y_offset
				}
				assert(off1 == 0);
				assert(off2 == 2);
				return true;
			}
		}
	
// ----
// f() -> 0x1
// z() -> 0x07

