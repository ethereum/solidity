pragma experimental "v0.5.0";

contract Base {
  constructor(uint) public {}
}
contract Derived is Base(2) { }
contract Derived2 is Base(), Derived() { }
// ----
// TypeError: Wrong argument count for constructor call: 0 arguments given but expected 1.
