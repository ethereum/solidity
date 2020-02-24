
		contract C {
			function g() public returns (uint a, uint b, uint c) {
				a = 1; b = 2; c = 3;
			}
			function h() public returns (uint a, uint b, uint c, uint d) {
				a = 1; b = 2; c = 3; d = 4;
			}
			function f1() public returns (bool) {
				(uint x, uint y, uint z) = g();
				if (x != 1 || y != 2 || z != 3) return false;
				(, uint a,) = g();
				if (a != 2) return false;
				(uint b, , ) = g();
				if (b != 1) return false;
				(, , uint c) = g();
				if (c != 3) return false;
				return true;
			}
			function f2() public returns (bool) {
				(uint a1, , uint a3, ) = h();
				if (a1 != 1 || a3 != 3) return false;
				(uint b1, uint b2, , ) = h();
				if (b1 != 1 || b2 != 2) return false;
				(, uint c2, uint c3, ) = h();
				if (c2 != 2 || c3 != 3) return false;
				(, , uint d3, uint d4) = h();
				if (d3 != 3 || d4 != 4) return false;
				(uint e1, , uint e3, uint e4) = h();
				if (e1 != 1 || e3 != 3 || e4 != 4) return false;
				return true;
			}
			function f() public returns (bool) {
				return f1() && f2();
			}
		}
	
// ----
// f() -> true

