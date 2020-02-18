
		contract c {
			struct S { uint a; uint b; }
			S public x;
			S public y;
			function set() public {
				x.a = 1; x.b = 2;
				y.a = 3; y.b = 4;
			}
			function swap() public {
				(x, y) = (y, x);
			}
		}
	
// ----
// x() -> 0x00, 0x00
// y() -> 0x00, 0x00
// set() -> 
// x() -> 0x01, 0x02
// y() -> 0x03, 0x04
// swap() -> 
// x() -> 0x01, 0x02
// y() -> 0x01, 0x02

