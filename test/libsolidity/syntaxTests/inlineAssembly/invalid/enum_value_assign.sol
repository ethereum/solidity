contract C {
	enum A { X, Y, Z }
	function f() public pure {
		assembly {
			A.X := 2
		}
	}
}
// ----
// TypeError: (77-80): Enum values cannot be assigned to.
