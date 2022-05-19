contract C {
  function f() public pure returns (uint r) {
    assembly { function f() -> x { x := 1 } r := f() }
  }
  function g() public pure returns (uint r) {
    assembly { function f() -> x { x := 2 } r := f() }
  }
}
// ----
// f() -> 1
// g() -> 2
