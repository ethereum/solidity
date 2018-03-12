contract test {
	function f() pure public {
		uint256 x;
		if (true) { uint256 x; }
	}
}
// ----
// DeclarationError: Identifier already declared.
