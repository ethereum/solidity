contract Base {
  constructor(uint) public {}
}
contract Base2 {
  constructor(uint, uint) public {}
}
contract Derived is Base(2) { }
contract Derived2 is Base(), Derived() { }
contract Derived3 is Base2(2,2) { }
contract Derived4 is Base2(2), Derived() { }
// ----
// TypeError: (156-162): Wrong argument count for constructor call: 0 arguments given but expected 1.
// TypeError: (235-243): Wrong argument count for constructor call: 1 arguments given but expected 2.
