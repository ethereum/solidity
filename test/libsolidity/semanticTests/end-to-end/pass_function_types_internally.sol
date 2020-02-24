
		contract C {
			function f(uint x) public returns (uint) {
				return eval(g, x);
			}
			function eval(function(uint) internal returns (uint) x, uint a) internal returns (uint) {
				return x(a);
			}
			function g(uint x) public returns (uint) { return x + 1; }
		}
	
// ----
// f(uint256): 7 -> 8

