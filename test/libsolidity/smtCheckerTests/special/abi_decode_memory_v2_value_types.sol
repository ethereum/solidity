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
// Warning: (32-67): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (125-132): Unused local variable.
// Warning: (183-190): Unused local variable.
// Warning: (136-139): Assertion checker does not yet implement type abi
// Warning: (136-167): Assertion checker does not yet implement this type of function call.
// Warning: (194-197): Assertion checker does not yet implement type abi
// Warning: (194-225): Assertion checker does not yet implement this type of function call.
// Warning: (303-319): Assertion violation happens here
