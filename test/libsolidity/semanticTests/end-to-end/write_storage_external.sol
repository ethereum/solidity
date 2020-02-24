
		contract C {
			uint public x;
			function f(uint y) public payable {
				x = y;
			}
			function g(uint y) external {
				x = y;
			}
			function h() public {
				this.g(12);
			}
		}
		contract D {
			C c = new C();
			function f() public payable returns (uint) {
				c.g(3);
				return c.x();
			}
			function g() public returns (uint) {
				c.g(8);
				return c.x();
			}
			function h() public returns (uint) {
				c.h();
				return c.x();
			}
		}
	
// ----
// f() -> 3
// g() -> 8
// h() -> 12

