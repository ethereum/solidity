
		contract C {
			function f() public {
				assert(false);
			}
			function g(bool val) public returns (bool) {
				assert(val == true);
				return true;
			}
			function h(bool val) public returns (bool) {
				require(val);
				return true;
			}
		}
	
// ====
// compileViaYul: also
// ----
// f() -> 
// g(bool): false -> 
// g(bool): true -> true
// h(bool): false -> 
// h(bool): true -> true

