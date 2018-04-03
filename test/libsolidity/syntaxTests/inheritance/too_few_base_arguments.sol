contract Base {
  constructor(uint, uint) public {}
}
contract Derived is Base(2) { }
contract Derived2 is Base {
  constructor() Base(2) public { }
}
// ----
// TypeError: Wrong argument count for constructor call: 1 arguments given but expected 2.
// TypeError: Wrong argument count for modifier invocation: 1 arguments given but expected 2.
