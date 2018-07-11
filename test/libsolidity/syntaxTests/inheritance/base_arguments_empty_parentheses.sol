contract Base {
  constructor(uint) public {}
}
contract Derived is Base(2) { }
contract Derived2 is Base(), Derived() { }
// ----
// TypeError: (101-107): Wrong argument count for constructor call: 0 arguments given but expected 1. Remove parentheses if you do not want to provide arguments here.
