
		contract C {
			function () internal returns (uint) x;
			constructor() public {
				x = unused;
			}
			function unused() internal returns (uint) {
				return 7;
			}
			function t() public returns (uint) {
				return x();
			}
		}
	
// ----
// t() -> 7

