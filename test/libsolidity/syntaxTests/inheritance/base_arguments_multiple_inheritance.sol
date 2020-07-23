contract Base {
    constructor(uint) { }
}
contract Base1 is Base(3) {}
contract Derived is Base, Base1 {
    constructor(uint i) Base(i) {}
}
// ----
// DeclarationError 3364: (131-138): Base constructor arguments given twice.
