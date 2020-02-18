
		library L { function x() internal returns (uint) { return 7; } }
		contract C {
			function () internal returns (uint) x;
			constructor() public {
				x = L.x;
			}
			function t() public returns (uint) {
				return x();
			}
		}
	
// ----
// t() -> 0x07

