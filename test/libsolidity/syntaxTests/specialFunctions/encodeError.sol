interface I {
	error IErr(uint256 p, string t);
}

contract Other {
	error OtherErr(uint);
}

library L {
	error LErr(uint256 p, string t);
}

contract Base {
	error BaseErr(uint);
}

contract C is Base {
	error Err(int a);
	error Err2(int a, string b);
	error Err4();

	function successErrorArgsIntLiteralTuple() public pure returns(bytes memory) {
		return abi.encodeError(Err, (1));
	}
	function successErrorArgsIntLiteral() public pure returns(bytes memory) {
		return abi.encodeError(Err, 1);
	}
	function successErrorArgsLiteralTuple() public pure returns(bytes memory) {
		return abi.encodeError(Err2, (1, "test"));
	}
	function successErrorArgsEmptyTuple() public pure returns(bytes memory) {
		return abi.encodeError(Err4, ());
	}
	function viaDeclaration() public pure returns (bytes memory) {
		return bytes.concat(
			abi.encodeError(I.IErr, (1, "234")),
			abi.encodeError(Other.OtherErr, (1)),
			abi.encodeError(L.LErr, (1, "123"))
		);
	}
	function viaBaseDeclaration() public pure returns (bytes memory) {
		return abi.encodeError(Base.BaseErr, (1));
	}
}
// ----
