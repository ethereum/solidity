contract C { constructor(uint) public {} }
contract A is C(2) {}
contract B is C(2) {}
contract D is A, B {}
// ----
// Warning: Base constructor arguments given twice.
