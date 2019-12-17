{
  function f() -> x, y
  {
    for { x := 0 } lt(x, 10) { x := add(x, 1) } {
      if eq(x, 5) { y := 1 leave }
    }
    x := 9
  }
  {
    let a, b := f()
    sstore(a, b)
  }
}
// ====
// EVMVersion: >=constantinople
// ----
// Trace:
// Memory dump:
// Storage dump:
//   0000000000000000000000000000000000000000000000000000000000000005: 0000000000000000000000000000000000000000000000000000000000000001
