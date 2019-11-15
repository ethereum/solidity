pragma experimental SMTChecker;
pragma experimental "ABIEncoderV2";

contract C {
  struct S { uint x; uint[] b; }
  function f() public pure returns (S memory, bytes memory) {
    return abi.decode("abc", (S, bytes));
  }
}
// ----
// Warning: (32-67): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (151-159): Assertion checker does not yet support the type of this variable.
// Warning: (188-191): Assertion checker does not yet implement type abi
// Warning: (207-208): Assertion checker does not yet implement type type(struct C.S storage pointer)
// Warning: (188-217): Assertion checker does not yet implement this type of function call.
