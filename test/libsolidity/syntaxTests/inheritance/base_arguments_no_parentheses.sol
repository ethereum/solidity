contract Base {
  constructor(uint) {}
}
contract Derived is Base(2) { }
contract Derived2 is Base, Derived {}
