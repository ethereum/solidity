contract C {
  bool nc = false;
  bool constant c = nc;
  function f() public {
    assembly {
        let t := c
    }
  }
}
// ----
// TypeError: (52-54): Initial value for constant variable has to be compile-time constant.
// TypeError: (112-113): Only direct number constants and references to such constants are supported by inline assembly.
