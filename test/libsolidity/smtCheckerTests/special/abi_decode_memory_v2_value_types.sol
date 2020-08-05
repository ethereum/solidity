pragma experimental SMTChecker;
pragma experimental "ABIEncoderV2";

contract C {
  function f() public pure {
    (uint x1, bool b1) = abi.decode("abc", (uint, bool));
    (uint x2, bool b2) = abi.decode("abc", (uint, bool));
	// False positive until abi.* are implemented as uninterpreted functions.
	assert(x1 == x2);
  }
}
// ----
// Warning 2072: (125-132): Unused local variable.
// Warning 2072: (183-190): Unused local variable.
// Warning 6328: (303-319): Assertion violation happens here
// Warning 8364: (136-139): Assertion checker does not yet implement type abi
// Warning 4588: (136-167): Assertion checker does not yet implement this type of function call.
// Warning 8364: (194-197): Assertion checker does not yet implement type abi
// Warning 4588: (194-225): Assertion checker does not yet implement this type of function call.
