
		contract test {
			function f(bool cond) public returns (bytes32) {
				return cond ? "true" : "false";
			}
		}
	
// ----
// f(bool): true -> true, 4
// f(bool): false -> false, 5

