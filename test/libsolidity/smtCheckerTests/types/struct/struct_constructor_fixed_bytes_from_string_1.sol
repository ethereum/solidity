pragma experimental SMTChecker;
contract C {
  struct S {
    int a;
    bytes5 b;
  }
  function f() public pure {
    assert(S({a:2, b:""}).b == bytes5(0)); // should hold
    assert(S({a:2, b:""}).a == 0); // should fail
  }
}
// ----
// Warning 5523: (0-31): The SMTChecker pragma has been deprecated and will be removed in the future. Please use the "model checker engine" compiler setting to activate the SMTChecker instead. If the pragma is enabled, all engines will be used.
// Warning 6328: (178-207): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.f()
