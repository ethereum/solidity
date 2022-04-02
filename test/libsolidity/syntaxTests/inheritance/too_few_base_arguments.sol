contract Base {
  constructor(uint, uint) {}
}
contract Derived is Base(2) { }
contract Derived2 is Base {
  constructor() Base(2) { }
}
// ----
// TypeError 7927: (67-74='Base(2)'): Wrong argument count for constructor call: 1 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 2973: (123-130='Base(2)'): Wrong argument count for modifier invocation: 1 arguments given but expected 2.
