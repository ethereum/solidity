
		contract C {
			function f(int a, int b) public returns (int) {
				return a << b;
			}
			function g(int a, int b) public returns (int) {
				return a >> b;
			}
		}
	
// ----
// f(int256,int256): 1, -1 -> 
// g(int256,int256): 1, -1 -> 

