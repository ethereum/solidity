contract C { constructor(uint) public {} }
contract A is C(2) {}
contract B is C(2) {}
contract D is A, B { constructor() C(3) public {} }
// ----
// Warning: Duplicated super constructor calls are deprecated.
// Warning: Duplicated super constructor calls are deprecated.
