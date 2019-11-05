contract D {
    enum B { V }
}
contract C {
	enum A { X, Y, Z }
	function f() public pure returns (uint a, uint b, uint c, uint d, uint e) {
		assembly {
			a := A.X
			b := C.A.Y
			c := D.B.V
			{
				let A.X := 2
				d := A.X
				let A.A := 3
				e := A.A
			}
		}
	}
}
