pragma experimental SMTChecker;
contract C { constructor(uint) public {} }
contract A is C { constructor() C(2) public {} }
// ----
// Warning: (45-72): Assertion checker does not yet support constructors.
// Warning: (93-121): Assertion checker does not yet support constructors.
