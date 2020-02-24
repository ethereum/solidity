
		contract C {
			bytes32 public x;
			uint public a;
			function f(bytes32 _x, uint _a) public {
				x = _x;
				a = _a;
			}
			function g() public {
				this.f("", 2);
			}
		}
	
// ----
// x() -> 0
// a() -> 0
// g() -> 
// x() -> 0
// a() -> 2

