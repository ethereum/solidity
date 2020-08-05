contract C {
  function f() public pure {
    assembly {
      pop(add(add(1, 2), c))
    }
  }
  int constant c = 1;
}
