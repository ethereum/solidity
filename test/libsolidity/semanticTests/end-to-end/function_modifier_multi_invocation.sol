
		contract C {
			modifier repeat(bool twice) { if (twice) _; _; }
			function f(bool twice) repeat(twice) public returns (uint r) { r += 1; }
		}
	
// ----
// f(bool): false -> 1
// f(bool): true -> 2

