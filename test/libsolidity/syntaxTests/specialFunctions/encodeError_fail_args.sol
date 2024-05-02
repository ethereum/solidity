contract C {
	error f(int a);
	error f3(int a, int b);

	function failErrorArgsWrongType() public returns(bytes memory) {
		return abi.encodeError(f, ("test"));
	}
	function failErrorArgsTooMany() public returns(bytes memory) {
		return abi.encodeError(f, (1, 2));
	}
	function failErrorArgsTooFew0() public returns(bytes memory) {
		return abi.encodeError(f, ());
	}
	function failErrorArgsTooFew1() public returns(bytes memory) {
		return abi.encodeError(f);
	}
	function failErrorArgsArrayLiteral() public returns(bytes memory) {
		return abi.encodeError(f3, [1, 2]);
	}
}
// ----
// TypeError 5407: (150-158): Cannot implicitly convert component at position 0 from "literal_string "test"" to "int256".
// TypeError 7788: (237-263): Expected 1 instead of 2 components for the tuple parameter.
// TypeError 7788: (341-363): Expected 1 instead of 0 components for the tuple parameter.
// TypeError 6220: (441-459): Expected two arguments: a custom error followed by a tuple.
// TypeError 7515: (542-569): Expected a tuple with 2 components instead of a single non-tuple parameter.
// TypeError 5407: (562-568): Cannot implicitly convert component at position 0 from "uint8[2] memory" to "int256".
