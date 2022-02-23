{
  function f(x) -> y {
    if lt(x, 150) {
      y := f(add(x, 1))
    }
    if eq(x, 150) {
      y := x
    }
  }
  mstore(0, f(0))
}
// ----
// Trace:
// Memory dump:
//      0: 0000000000000000000000000000000000000000000000000000000000000096
// Storage dump:
