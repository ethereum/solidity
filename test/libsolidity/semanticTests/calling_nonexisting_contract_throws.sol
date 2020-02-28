
		abstract contract D { function g() public virtual; }
		contract C {
			D d = D(0x1212);
			function f() public returns (uint) {
				d.g();
				return 7;
			}
			function g() public returns (uint) {
				d.g.gas(200)();
				return 7;
			}
			function h() public returns (uint) {
				address(d).call(""); // this does not throw (low-level)
				return 7;
			}
		}
	
// ----
// f() -> 
// g() -> 
// h() -> 7

