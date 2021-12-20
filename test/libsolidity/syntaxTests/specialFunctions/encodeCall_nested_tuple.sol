contract C {
	function f(int a, int b) public {}
	function failFunctionArgsIntLiteralNestedTuple() public returns(bytes memory) {
		return abi.encodeCall(this.f, ((1,2)));
	}
}
// ----
// TypeError 7788: (139-170): Expected 2 instead of 1 components for the tuple parameter.
// TypeError 5407: (163-168): Cannot implicitly convert component at position 0 from "tuple(int_const 1,int_const 2)" to "int256".
