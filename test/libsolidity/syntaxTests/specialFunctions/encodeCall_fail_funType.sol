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
	function baseFunctionInternal(uint) internal pure {}
	function baseFunctionPublic(uint) public pure {}
}

function fileLevel(uint) pure {}

contract C is Base {
	using L for uint256;

	function fPublic(int a) public {}
	function fInternal(uint256 p, string memory t) internal {}

	function failFunctionPtrMissing() public returns(bytes memory) {
		return abi.encodeCall(1, this.fPublic);
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
	function failLibraryPointerCall() public {
		abi.encodeCall(L.fInternal, (1, "123"));
		abi.encodeCall(L.fExternal, (1, "123"));
	}
	function failAttachedLibraryPointerCall() public returns (bytes memory) {
		uint256 x = 1;
		return abi.encodeCall(x.fExternal, (1, "123"));
	}
	function viaBaseDeclaration() public pure returns (bytes memory) {
		return abi.encodeCall(C.fPublic, (2));
	}
	function viaBaseDeclaration2() public pure returns (bytes memory) {
		return bytes.concat(
			abi.encodeCall(Base.baseFunctionPublic, (1)),
			abi.encodeCall(Base.baseFunctionInternal, (1))
		);
	}
	function fileLevelFunction() public pure returns (bytes memory) {
		return abi.encodeCall(fileLevel, (2));
	}
	function createFunction() public pure returns (bytes memory) {
		return abi.encodeCall(new Other, (2));
	}
}
// ----
// TypeError 5511: (742-743): Expected first argument to be a function pointer, not "int_const 1".
// TypeError 3509: (855-869): Expected regular external function type, or external view on public function. Cannot use special function.
// TypeError 3509: (982-991): Expected regular external function type, or external view on public function. Provided internal function.
// TypeError 3509: (1187-1207): Expected regular external function type, or external view on public function. Provided internal function.
// TypeError 3509: (1286-1297): Expected regular external function type, or external view on public function. Provided internal function.
// TypeError 3509: (1329-1340): Expected regular external function type, or external view on public function. Cannot use library functions for abi.encodeCall.
// TypeError 3509: (1474-1485): Expected regular external function type, or external view on public function. Cannot use library functions for abi.encodeCall.
// TypeError 3509: (1595-1604): Expected regular external function type, or external view on public function. Provided internal function. Did you forget to prefix "this."?
// TypeError 3509: (1725-1748): Expected regular external function type, or external view on public function. Provided internal function. Functions from base contracts have to be external.
// TypeError 3509: (1774-1799): Expected regular external function type, or external view on public function. Provided internal function. Functions from base contracts have to be external.
// TypeError 3509: (1905-1914): Expected regular external function type, or external view on public function. Provided internal function.
// TypeError 3509: (2013-2022): Expected regular external function type, or external view on public function. Provided creation function.
