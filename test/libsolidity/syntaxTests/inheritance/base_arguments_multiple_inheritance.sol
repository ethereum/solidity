contract Base {
    constructor(uint) public { }
}
contract Base1 is Base(3) {}
contract Derived is Base, Base1 {
    constructor(uint i) Base(i) public {}
}
// ----
// DeclarationError: (138-145): Base constructor arguments given twice.
