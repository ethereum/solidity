contract Base {
  function Base(uint, uint) public {}
}
contract Derived is Base(2) { }
contract Derived2 is Base {
  function Derived2() Base(2) public { }
}
// ----
// TypeError: Wrong argument count for constructor call: 1 arguments given but expected 2.
// TypeError: Wrong argument count for modifier invocation: 1 arguments given but expected 2.
