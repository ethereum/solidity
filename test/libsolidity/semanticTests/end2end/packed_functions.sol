
		contract C {
			// these should take the same slot
			function() internal returns (uint) a;
			function() external returns (uint) b;
			function() external returns (uint) c;
			function() internal returns (uint) d;
			uint8 public x;

			function set() public {
				x = 2;
				d = g;
				c = this.h;
				b = this.h;
				a = g;
			}
			function t1() public returns (uint) {
				return a();
			}
			function t2() public returns (uint) {
				return b();
			}
			function t3() public returns (uint) {
				return a();
			}
			function t4() public returns (uint) {
				return b();
			}
			function g() public returns (uint) {
				return 7;
			}
			function h() public returns (uint) {
				return 8;
			}
		}
	
// ----
// set() -> 
// t1() -> 0x07
// t2() -> 0x08
// t3() -> 0x07
// t4() -> 0x08
// x() -> 0x02

