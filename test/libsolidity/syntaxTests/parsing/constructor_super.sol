contract A {
  function x() pure internal {}
}

contract B is A {
  constructor() public {
    // used to trigger warning about using ``this`` in constructor
    super.x();
  }
}
