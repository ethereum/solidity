
		library L {
			struct S { uint[] data; }
			function f(S memory _s) internal returns (uint[] memory) {
				_s.data[3] = 2;
				return _s.data;
			}
		}
		contract C {
			using L for L.S;
			function f() public returns (uint) {
				L.S memory x;
				x.data = new uint[](7);
				x.data[3] = 8;
				return x.f()[3];
			}
		}
	
// ----
// f() -> 0x02

