interface I {
	error Err(uint256 p, string t);
}

contract Other {
	error Err(uint);
}

library L {
	error Err(uint256 p, string t);
}

contract Base {
	error BaseErr(uint);
}

error fileLevel(uint);

contract C is Base {
	using L for uint256;

	error Err(int a);

	function failErrorPtrMissing() public returns(bytes memory) {
		return abi.encodeError(1, Err);
	}
	function failErrorPtrWrongType() public returns(bytes memory) {
		return abi.encodeError(abi.encodeCall, (1, 2, 3, "test"));
	}
	function viaBaseDeclaration() public pure returns (bytes memory) {
		return abi.encodeError(C.Err, (1));
	}
	function viaBaseDeclaration2() public pure returns (bytes memory) {
		return abi.encodeError(Base.BaseErr, (1));
	}
	function viaOther() public pure returns (bytes memory) {
		return abi.encodeError(Other.Err, (1));
	}
	function viaInterface() public pure returns (bytes memory) {
		return abi.encodeError(I.Err, (1, "123"));
	}
	function viaLibrary() public pure returns (bytes memory) {
		return abi.encodeError(L.Err, (1, "123"));
	}
	function fileLevelFunction() public pure returns (bytes memory) {
		return abi.encodeError(fileLevel, (2));
	}
	function createFunction() public pure returns (bytes memory) {
		return abi.encodeError(new Other, (2));
	}
}
// ----
// TypeError 5512: (353-354): Expected first argument to be a custom error, not "int_const 1".
// TypeError 3510: (455-469): Expected an error type. Cannot use special function.
// TypeError 3510: (1242-1251): Expected an error type. Provided creation function.
