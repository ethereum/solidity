struct Struct { uint x; }

library L {
    function f(Struct storage _x) internal view returns (uint256) {
      return _x.x;
    }
}

contract C {
  using L for Struct;

  Struct s;

  function h(Struct storage _s) internal view returns (uint) {
    // _s is pointer
    return _s.f();
  }
  function g() public returns (uint, uint) {
    s.x = 7;
    // s is reference
    return (s.f(), h(s));
  }
}
// ====
// compileViaYul: also
// ----
// g() -> 7, 7
