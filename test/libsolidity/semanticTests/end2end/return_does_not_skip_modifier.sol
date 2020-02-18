
		contract C {
			uint public x;
			modifier setsx {
				_;
				x = 9;
			}
			function f() setsx public returns (uint) {
				return 2;
			}
		}
	
// ----
// x() -> 0x00
// f() -> 0x02
// x() -> 0x09

