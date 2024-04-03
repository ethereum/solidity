contract C {
	error e(int a);
	function failErrorArgsIntLiteralTuple() public returns(bytes memory) {
		return abi.encodeError(e, (1,));
	}
}
// ----
// TypeError 8381: (130-134): Tuple component cannot be empty.
