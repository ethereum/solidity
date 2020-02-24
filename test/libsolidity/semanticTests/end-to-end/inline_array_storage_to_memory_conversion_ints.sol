
		contract C {
			function f() public returns (uint x, uint y) {
				x = 3;
				y = 6;
				uint[2] memory z = [x, y];
				return (z[0], z[1]);
			}
		}
	
// ----
// f() -> 3, 6

