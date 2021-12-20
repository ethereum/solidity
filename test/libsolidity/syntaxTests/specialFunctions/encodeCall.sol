interface I {
	function fExternal(uint256 p, string memory t) external;
}

library L {
	function fExternal(uint256 p, string memory t) external {}
}

contract C {
	using L for uint256;

	function f(int a) public {}
	function f2(int a, string memory b) public {}
	function f3(int a, int b) public {}
	function f4() public {}
	function fInternal(uint256 p, string memory t) internal {}

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
	function failFunctionPtrMissing() public returns(bytes memory) {
		return abi.encodeCall(1, this.f);
	}
	function failFunctionPtrWrongType() public returns(bytes memory) {
		return abi.encodeCall(abi.encodeCall, (1, 2, 3, "test"));
	}
	function failFunctionInternal() public returns(bytes memory) {
		return abi.encodeCall(fInternal, (1, "123"));
	}
	function failFunctionInternalFromVariable() public returns(bytes memory) {
		function(uint256, string memory) internal localFunctionPointer = fInternal;
		return abi.encodeCall(localFunctionPointer, (1, "123"));
	}
	function failFunctionArgsArrayLiteral() public returns(bytes memory) {
		return abi.encodeCall(this.f3, [1, 2]);
	}
	function failLibraryPointerCall() public returns (bytes memory) {
		return abi.encodeCall(L.fExternal, (1, "123"));
	}
	function failBoundLibraryPointerCall() public returns (bytes memory) {
		uint256 x = 1;
		return abi.encodeCall(x.fExternal, (1, "123"));
	}
	function failInterfacePointerCall() public returns (bytes memory) {
		return abi.encodeCall(I.fExternal, (1, "123"));
	}
	function successFunctionArgsIntLiteralTuple() public returns(bytes memory) {
		return abi.encodeCall(this.f, (1));
	}
	function successFunctionArgsIntLiteral() public returns(bytes memory) {
		return abi.encodeCall(this.f, 1);
	}
	function successFunctionArgsLiteralTuple() public returns(bytes memory) {
		return abi.encodeCall(this.f2, (1, "test"));
	}
	function successFunctionArgsEmptyTuple() public returns(bytes memory) {
		return abi.encodeCall(this.f4, ());
	}
}
// ----
// TypeError 5407: (486-494): Cannot implicitly convert component at position 0 from "literal_string "test"" to "int256".
// TypeError 7788: (576-606): Expected 1 instead of 2 components for the tuple parameter.
// TypeError 7788: (687-713): Expected 1 instead of 0 components for the tuple parameter.
// TypeError 6219: (794-816): Expected two arguments: a function pointer followed by a tuple.
// TypeError 5511: (911-912): Expected first argument to be a function pointer, not "int_const 1".
// TypeError 3509: (1018-1032): Function must be "public" or "external".
// TypeError 3509: (1145-1154): Function must be "public" or "external". Did you forget to prefix "this."?
// TypeError 3509: (1350-1370): Function must be "public" or "external".
// TypeError 7515: (1469-1500): Expected a tuple with 2 components instead of a single non-tuple parameter.
// TypeError 5407: (1493-1499): Cannot implicitly convert component at position 0 from "uint8[2]" to "int256".
// TypeError 3509: (1596-1607): Function must be "public" or "external".
// TypeError 3509: (1738-1749): Function must be "public" or "external".
// TypeError 3509: (1860-1871): Function must be "public" or "external".
