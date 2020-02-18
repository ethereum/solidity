
		contract C {
			struct S { uint x; }
			S s;
			function g() internal returns (uint, S storage, uint) {
				s.x = 7;
				return (1, s, 2);
			}
			function f() public returns (bool) {
				(uint x1, S storage y1, uint z1) = g();
				if (x1 != 1 || y1.x != 7 || z1 != 2) return false;
				(, S storage y2,) = g();
				if (y2.x != 7) return false;
				(uint x2,,) = g();
				if (x2 != 1) return false;
				(,,uint z2) = g();
				if (z2 != 2) return false;
				return true;
			}
		}
	
// ----
// f(): hex"" -> 0x1

