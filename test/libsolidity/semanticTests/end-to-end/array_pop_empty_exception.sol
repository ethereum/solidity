
		contract c {
			uint[] data;
			function test() public returns (bool) {
				data.pop();
				return true;
			}
		}
	
// ----
// test() -> FAILURE
