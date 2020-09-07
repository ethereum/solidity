pragma experimental SMTChecker;
pragma experimental "ABIEncoderV2";

contract C {
  struct S { uint x; uint[] b; }
  function f() public pure returns (S memory, bytes memory, uint[][2] memory) {
    return abi.decode("abc", (S, bytes, uint[][2]));
  }
}
// ----
// Warning 8364: (206-209): Assertion checker does not yet implement type abi
// Warning 8364: (225-226): Assertion checker does not yet implement type type(struct C.S storage pointer)
// Warning 8364: (235-241): Assertion checker does not yet implement type type(uint256[] memory)
// Warning 8364: (235-244): Assertion checker does not yet implement type type(uint256[] memory[2] memory)
// Warning 4588: (206-246): Assertion checker does not yet implement this type of function call.
