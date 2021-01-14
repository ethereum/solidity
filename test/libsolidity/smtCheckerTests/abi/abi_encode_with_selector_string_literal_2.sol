pragma experimental SMTChecker;
contract C {
	function abiEncodeStringLiteral(bytes4 sel) public pure {
		bytes memory b1 = abi.encodeWithSelector("");
		require(sel == "");
		bytes memory b2 = abi.encodeWithSelector(sel);
		assert(b1.length == b2.length); // should hold
	}
}
