contract C {
	function f() {
		uint[] storage x;
		uint[10] storage y;
	}
}
// ----
// DeclarationError: (31-47): Uninitialized storage pointer.
// DeclarationError: (51-69): Uninitialized storage pointer.
