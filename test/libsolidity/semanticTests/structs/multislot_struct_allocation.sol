contract C {
  struct I {
    uint b;
    uint c;
    function(uint) external returns (uint) x;
  }
  struct S {
    I a;
  }

  function o(uint a) external returns(uint) { return a+1; }

  function f() external returns (uint) {
    S memory s = S(I(1,2, this.o));
    return s.a.x(1);
  }
}



// ====
// compileViaYul: also
// ----
// f() -> 2
