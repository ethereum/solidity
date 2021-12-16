contract C {
	function f(int a) public {}
	function failFunctionArgsIntLiteralTuple() public returns(bytes memory) {
		return abi.encodeCall(this.f, (1,));
	}
}
// ----
// TypeError 8381: (149-153): Tuple component cannot be empty.
