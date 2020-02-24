
		contract C {
			function intern() public returns (uint) {
				function (uint) internal returns (uint) x;
				x(2);
				return 7;
			}
			function extern() public returns (uint) {
				function (uint) external returns (uint) x;
				x(2);
				return 7;
			}
		}
	
// ----
// intern() -> FAILURE # This should throw exceptions #
// extern() -> FAILURE
