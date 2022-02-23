interface I {
	function fExternal(uint256 p, string memory t) external;
}

contract Other {
	function fExternal(uint) external pure {}
	function fPublic(uint) public pure {}
	function fInternal(uint) internal pure {}
}

library L {
	function fExternal(uint256 p, string memory t) external {}
	function fInternal(uint256 p, string memory t) internal {}
}

contract Base {
	function baseFunctionExternal(uint) external pure {}
}

contract C is Base {
	function f(int a) public {}
	function f2(int a, string memory b) public {}
	function f4() public {}

	function successFunctionArgsIntLiteralTuple() public view returns(bytes memory) {
		return abi.encodeCall(this.f, (1));
	}
	function successFunctionArgsIntLiteral() public view returns(bytes memory) {
		return abi.encodeCall(this.f, 1);
	}
	function successFunctionArgsLiteralTuple() public view returns(bytes memory) {
		return abi.encodeCall(this.f2, (1, "test"));
	}
	function successFunctionArgsEmptyTuple() public view returns(bytes memory) {
		return abi.encodeCall(this.f4, ());
	}
	function viaDeclaration() public pure returns (bytes memory) {
		return bytes.concat(
			abi.encodeCall(Other.fExternal, (1)),
			abi.encodeCall(Other.fPublic, (1)),
			abi.encodeCall(I.fExternal, (1, "123"))
		);
	}
	function viaBaseDeclaration() public pure returns (bytes memory) {
		return abi.encodeCall(Base.baseFunctionExternal, (1));
	}
}
// ----
