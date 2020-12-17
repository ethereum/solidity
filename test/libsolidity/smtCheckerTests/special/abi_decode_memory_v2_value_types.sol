pragma experimental SMTChecker;
pragma experimental "ABIEncoderV2";

contract C {
	function f(bytes memory data) public pure {
		(uint x1, bool b1) = abi.decode(data, (uint, bool));
		(uint x2, bool b2) = abi.decode(data, (uint, bool));
		assert(x1 == x2);
	}
}
// ----
// Warning 2072: (139-146): Unused local variable.
// Warning 2072: (194-201): Unused local variable.
