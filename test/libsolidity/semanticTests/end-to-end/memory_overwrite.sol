
		contract C {
			function f() public returns (bytes memory x) {
				x = "12345";
				x[3] = 0x61;
				x[0] = 0x62;
			}
		}
	
// ----
// f() -> "b23a5"

