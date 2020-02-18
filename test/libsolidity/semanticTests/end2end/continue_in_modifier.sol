
		contract C {
			uint public x;
			modifier run() {
				for (uint i = 0; i < 10; i++) {
					if (i % 2 == 1) continue;
					_;
				}
			}
			function f() run public {
				uint k = x;
				uint t = k + 1;
				x = t;
			}
		}
	
// ----
// x() -> 0x00
// f() -> 
// x() -> 0x05

