contract C {
	function f() public {
		uint[] storage x;
		uint[10] storage y;
	}
}
// ----
// DeclarationError: (38-54): Uninitialized storage pointer.
// DeclarationError: (58-76): Uninitialized storage pointer.
