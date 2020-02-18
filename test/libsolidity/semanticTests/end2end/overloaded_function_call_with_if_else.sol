
		contract test {
			function f(uint a, uint b) public returns(uint d) { return a + b; }
			function f(uint k) public returns(uint d) { return k; }
			function g(bool flag) public returns(uint d) {
				if (flag)
					return f(3);
				else
					return f(3, 7);
			}
		}
	
// ====
// optimize-yul: false
// ----
// g(bool): 0x1 -> 0x3
// g(bool): 0x0 -> 0xa

