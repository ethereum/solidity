
		contract C {
			struct S { uint[2] a; bytes b; }
			function f() public returns (byte, uint, uint, byte) {
				bytes memory x = new bytes(200);
				x[199] = 'A';
				uint[2][] memory y = new uint[2][](300);
				y[203][1] = 8;
				S[] memory z = new S[](180);
				z[170].a[1] = 4;
				z[170].b = new bytes(102);
				z[170].b[99] = 'B';
				return (x[199], y[203][1], z[170].a[1], z[170].b[99]);
			}
		}
	
// ----
// f() -> "A", 0x08, 0x04, "B"

