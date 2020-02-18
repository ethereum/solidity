
		contract Test {
			struct S { uint8 x; uint16 y; uint z; }
			struct X { uint8 x; S s; }
			function test() public returns (uint a, uint x, uint y, uint z) {
				X memory d = combine(1, 2, 3, 4);
				a = extract(d, 0);
				x = extract(d, 1);
				y = extract(d, 2);
				z = extract(d, 3);
			}
			function extract(X memory s, uint which) internal returns (uint x) {
				if (which == 0) return s.x;
				else if (which == 1) return s.s.x;
				else if (which == 2) return s.s.y;
				else return s.s.z;
			}
			function combine(uint8 a, uint8 x, uint16 y, uint z) internal returns (X memory s) {
				s.x = a;
				s.s.x = x;
				s.s.y = y;
				s.s.z = z;
			}
		}
	
// ----
// test() -> 0x01, 0x02, 0x03, 0x04

