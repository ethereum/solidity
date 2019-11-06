contract C {
	enum A { X, Y, Z }
	function f() public pure returns (uint a, uint b, uint c) {
		assembly {
			a := A.
			b := A.U
		}
	}
}
// ----
// DeclarationError: (115-117): Identifier not found.
// DeclarationError: (126-129): Identifier not found.
