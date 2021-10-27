contract C {
  uint x;
  modifier m() {
    uint t;
    assembly {
      function f() -> x { x := 8 }
      t := f()
    }
    x = t;
    _;
  }
  function f() m m public returns (uint r) {
    assembly { function f() -> x { x := 1 } r := f() }
  }
  function g() m m public returns (uint r) {
    assembly { function f() -> x { x := 2 } r := f() }
  }
}