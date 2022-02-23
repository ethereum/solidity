contract A {
    modifier m virtual {
      _;
    }
}
contract C is A {
    function f() public A.m returns (uint) {
    }
}
// ====
// SMTEngine: all
// ----
