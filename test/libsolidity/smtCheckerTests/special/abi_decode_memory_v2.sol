pragma experimental SMTChecker;
pragma experimental "ABIEncoderV2";

contract C {
	struct S { uint x; uint[] b; }
	function f() public pure returns (S memory, bytes memory, uint[][2] memory) {
		return abi.decode("abc", (S, bytes, uint[][2]));
	}
}
// ----
// Warning 8364: (231-237): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (231-240): Assertion checker does not yet implement type type(uint256[] memory[2] memory)
// Warning 8364: (221-222): Assertion checker does not yet implement type type(struct C.S storage pointer)
// Warning 4588: (202-242): Assertion checker does not yet implement this type of function call.
// Warning 8364: (231-237): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (231-240): Assertion checker does not yet implement type type(uint256[] memory[2] memory)
// Warning 8364: (221-222): Assertion checker does not yet implement type type(struct C.S storage pointer)
// Warning 4588: (202-242): Assertion checker does not yet implement this type of function call.
