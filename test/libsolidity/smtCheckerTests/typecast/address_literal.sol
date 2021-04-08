contract C {
  address x; // We know that this is "zero initialised".
  function f() public view {
    address a = address(0);
    assert(x == address(0));
    assert(x == a);
  }

  function g() public pure {
    address a = address(0);
    address b = address(1);
    address c = address(0);
    address d = a;
    address e = address(0x12345678);
    assert(c == d);
    assert(a == c);
    assert(e == address(305419896));
    // This is untrue.
    assert(a == b);
  }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (454-468): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\na = 0\nb = 1\nc = 0\nd = 0\ne = 305419896\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.g()
