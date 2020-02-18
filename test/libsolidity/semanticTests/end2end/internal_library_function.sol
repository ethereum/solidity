
		library L {
			function f(uint[] memory _data) internal {
				_data[3] = 2;
			}
		}
		contract C {
			function f() public returns (uint) {
				uint[] memory x = new uint[](7);
				x[3] = 8;
				L.f(x);
				return x[3];
			}
		}
	
// ----
// f() -> 0x02

