
		contract C {
			function f(int a, int b) public returns (int) {
				a <<= b;
				return a;
			}
			function g(int a, int b) public returns (int) {
				a >>= b;
				return a;
			}
		}
	
// ----
// f(int256,int256): 1, -1 -> 
// g(int256,int256): 1, -1 -> 

