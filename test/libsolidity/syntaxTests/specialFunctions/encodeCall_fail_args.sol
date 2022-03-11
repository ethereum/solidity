contract C {
	function f(int a) public {}
	function f3(int a, int b) public {}

	function failFunctionArgsWrongType() public returns(bytes memory) {
		return abi.encodeCall(this.f, ("test"));
	}
	function failFunctionArgsTooMany() public returns(bytes memory) {
		return abi.encodeCall(this.f, (1, 2));
	}
	function failFunctionArgsTooFew0() public returns(bytes memory) {
		return abi.encodeCall(this.f, ());
	}
	function failFunctionArgsTooFew1() public returns(bytes memory) {
		return abi.encodeCall(this.f);
	}
	function failFunctionArgsArrayLiteral() public returns(bytes memory) {
		return abi.encodeCall(this.f3, [1, 2]);
	}
}
// ----
// TypeError 5407: (181-189): Cannot implicitly convert component at position 0 from "literal_string "test"" to "int256".
// TypeError 7788: (271-301): Expected 1 instead of 2 components for the tuple parameter.
// TypeError 7788: (382-408): Expected 1 instead of 0 components for the tuple parameter.
// TypeError 6219: (489-511): Expected two arguments: a function pointer followed by a tuple.
// TypeError 7515: (597-628): Expected a tuple with 2 components instead of a single non-tuple parameter.
// TypeError 5407: (621-627): Cannot implicitly convert component at position 0 from "uint8[2] memory" to "int256".
