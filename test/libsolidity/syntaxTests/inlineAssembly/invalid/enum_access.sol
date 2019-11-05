contract C {
	enum A { X, Y, Z }
	function f() public pure returns (uint a) {
		assembly {
			a := A.A
		}
	}
}
// ----
// DeclarationError: (99-102): Identifier not found.
