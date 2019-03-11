contract C {
	function f() public {
		uint[] calldata x;
		uint[10] calldata y;
	}
}
// ----
// DeclarationError: (38-55): Uninitialized calldata pointer.
// DeclarationError: (59-78): Uninitialized calldata pointer.
