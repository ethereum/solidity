contract C {
  function f() public pure {
    assembly {
      let x := C
    }
  }
}
// ----
// TypeError 4977: (72-73='C'): Expected a library.
