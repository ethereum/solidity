
		contract C {
			modifier repeat(bool twice) { if (twice) _; _; }
			function f(bool twice) repeat(twice) public returns (uint r) { r += 1; return r; }
		}
	
// ----
// f(bool): 0x0 -> 0x1
// f(bool): 0x1 -> 0x2

