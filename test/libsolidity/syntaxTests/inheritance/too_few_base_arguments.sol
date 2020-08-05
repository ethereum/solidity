contract Base {
  constructor(uint, uint) {}
}
contract Derived is Base(2) { }
contract Derived2 is Base {
  constructor() Base(2) { }
}
// ----
// TypeError 7927: (67-74): Wrong argument count for constructor call: 1 arguments given but expected 2. Remove parentheses if you do not want to provide arguments here.
// TypeError 2973: (123-130): Wrong argument count for modifier invocation: 1 arguments given but expected 2.
