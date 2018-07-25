contract C {
  function f() public pure {
    assembly {
      let x := C
    }
  }
}
// ----
// TypeError: (72-73): Expected a library.
