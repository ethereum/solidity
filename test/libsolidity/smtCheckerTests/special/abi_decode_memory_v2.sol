pragma abicoder v2;

contract C {
	struct S { uint x; uint[] b; }
	function f() public pure returns (S memory, bytes memory, uint[][2] memory) {
		return abi.decode("abc", (S, bytes, uint[][2]));
	}
}
// ====
// SMTEngine: all
// ----
// Warning 8364: (183-189): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (183-192): Assertion checker does not yet implement type type(uint256[] memory[2] memory)
// Warning 8364: (173-174): Assertion checker does not yet implement type type(struct C.S storage pointer)
// Warning 8364: (183-189): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (183-192): Assertion checker does not yet implement type type(uint256[] memory[2] memory)
// Warning 8364: (173-174): Assertion checker does not yet implement type type(struct C.S storage pointer)
