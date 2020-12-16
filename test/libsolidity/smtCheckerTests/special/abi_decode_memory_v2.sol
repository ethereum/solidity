pragma experimental SMTChecker;
pragma abicoder v2;

contract C {
	struct S { uint x; uint[] b; }
	function f() public pure returns (S memory, bytes memory, uint[][2] memory) {
		return abi.decode("abc", (S, bytes, uint[][2]));
	}
}
// ----
// Warning 8364: (215-221): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (215-224): Assertion checker does not yet implement type type(uint256[] memory[2] memory)
// Warning 8364: (205-206): Assertion checker does not yet implement type type(struct C.S storage pointer)
// Warning 8364: (215-221): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (215-224): Assertion checker does not yet implement type type(uint256[] memory[2] memory)
// Warning 8364: (205-206): Assertion checker does not yet implement type type(struct C.S storage pointer)
