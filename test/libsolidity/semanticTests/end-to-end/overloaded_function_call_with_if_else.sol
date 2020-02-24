
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
// compileViaYul: also
// ----
// g(bool): true -> 3
// g(bool): false -> 10

