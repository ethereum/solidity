// This caused a failure since the type was not converted to its mobile type.
		contract C {
			function f() public returns (uint) {
				return [4][0];
			}
		}
	
// ----
// f() -> 4

