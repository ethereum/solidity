contract Base {
  constructor(uint8) public {}
}
contract Derived is Base(300) { }
contract Derived2 is Base {
  constructor() Base(2) public { }
}
// ----
// TypeError: (74-77): Invalid type for argument in constructor call. Invalid implicit conversion from int_const 300 to uint8 requested.
