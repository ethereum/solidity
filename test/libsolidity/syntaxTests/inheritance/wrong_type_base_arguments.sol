contract Base {
  constructor(uint8) {}
}
contract Derived is Base(300) { }
contract Derived2 is Base {
  constructor() Base(2) { }
}
// ----
// TypeError 9827: (67-70): Invalid type for argument in constructor call. Invalid implicit conversion from int_const 300 to uint8 requested. Literal is too large to fit in uint8.
