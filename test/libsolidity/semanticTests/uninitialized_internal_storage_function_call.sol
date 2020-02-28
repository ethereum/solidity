
		contract Test {
			function() internal x;
			function f() public returns (uint r) {
				x();
				return 2;
			}
		}
	
// ----
// f() -> 

