contract C {
  bool constant c = this;
  function f() public {
    assembly {
        let t := c
    }
  }
}
// ----
// TypeError 7407: (33-37='this'): Type contract C is not implicitly convertible to expected type bool.
// TypeError 8349: (33-37='this'): Initial value for constant variable has to be compile-time constant.
// TypeError 7615: (95-96='c'): Only direct number constants and references to such constants are supported by inline assembly.
