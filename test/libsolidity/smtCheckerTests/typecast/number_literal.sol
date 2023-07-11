contract C {
  function f() public pure {
    uint x = 1234;
    uint y = 0;
    assert(x != y);
    assert(x == uint(1234));
    assert(y == uint(0));
  }
  function g() public pure {
    uint a = uint(0);
    uint b = type(uint256).max;
    uint c = 115792089237316195423570985008687907853269984665640564039457584007913129639935;
    int d = -1;
    uint e = uint(d);
    assert(a != b);
    assert(b == c);
    assert(b == e);
  }
  function h() public pure {
    uint32 a = uint32(0);
    uint32 b = type(uint32).max;
    uint32 c = 4294967295;
    int32 d = -1;
    uint32 e = uint32(d);
    assert(a != b);
    assert(b == c);
    assert(b == e);
  }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 9 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
