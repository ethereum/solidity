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
// Warning 6328: (454-468): CHC: Assertion violation happens here.
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
