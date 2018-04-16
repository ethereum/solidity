contract test {
	function f() pure public {
		uint256 x;
		if (true) { uint256 x; }
	}
}
// ----
// DeclarationError: (71-80): Identifier already declared.
