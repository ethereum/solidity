pragma experimental SMTChecker;
contract C {
  function f() public pure {
    (uint a1, bytes32 b1, C c1) = abi.decode("abc", (uint, bytes32, C));
    (uint a2, bytes32 b2, C c2) = abi.decode("abc", (uint, bytes32, C));
	// False positive until abi.* are implemented as uninterpreted functions.
	assert(a1 == a2);
	assert(a1 != a2);
  }

}
// ----
// Warning: (88-98): Unused local variable.
// Warning: (100-104): Unused local variable.
// Warning: (161-171): Unused local variable.
// Warning: (173-177): Unused local variable.
// Warning: (108-111): Assertion checker does not yet implement type abi
// Warning: (142-143): Assertion checker does not yet implement type type(contract C)
// Warning: (108-145): Assertion checker does not yet implement this type of function call.
// Warning: (181-184): Assertion checker does not yet implement type abi
// Warning: (215-216): Assertion checker does not yet implement type type(contract C)
// Warning: (181-218): Assertion checker does not yet implement this type of function call.
// Warning: (296-312): Assertion violation happens here
