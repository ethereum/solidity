contract C {
	error e(int a, int b);
	function failErrorArgsIntLiteralNestedTuple() public returns(bytes memory) {
		return abi.encodeError(e, ((1,2)));
	}
}
// ----
// TypeError 7789: (124-151): Expected 2 instead of 1 components for the tuple parameter.
// TypeError 5408: (144-149): Cannot implicitly convert component at position 0 from "tuple(int_const 1,int_const 2)" to "int256".
