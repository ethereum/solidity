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
// Warning 2072: (88-98): Unused local variable.
// Warning 2072: (100-104): Unused local variable.
// Warning 2072: (161-171): Unused local variable.
// Warning 2072: (173-177): Unused local variable.
// Warning 6328: (296-312): Assertion violation happens here
// Warning 6328: (315-331): Assertion violation happens here
// Warning 8364: (108-111): Assertion checker does not yet implement type abi
// Warning 8364: (142-143): Assertion checker does not yet implement type type(contract C)
// Warning 4588: (108-145): Assertion checker does not yet implement this type of function call.
// Warning 8364: (181-184): Assertion checker does not yet implement type abi
// Warning 8364: (215-216): Assertion checker does not yet implement type type(contract C)
// Warning 4588: (181-218): Assertion checker does not yet implement this type of function call.
