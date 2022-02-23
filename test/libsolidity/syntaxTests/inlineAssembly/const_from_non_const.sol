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
// TypeError 8349: (52-54): Initial value for constant variable has to be compile-time constant.
// TypeError 7615: (112-113): Only direct number constants and references to such constants are supported by inline assembly.
