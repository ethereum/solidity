
		contract C {
			function() internal returns (uint)[20] x;
			int mutex;
			function one() public returns (uint) {
				function() internal returns (uint)[20] memory xmem;
				x = xmem;
				return 3;
			}
			function two() public returns (uint) {
				if (mutex > 0)
					return 7;
				mutex = 1;
				// If this test fails, it might re-execute this function.
				x[0]();
				return 2;
			}
		}
	
// ----
// one() -> 3
// two() -> 

