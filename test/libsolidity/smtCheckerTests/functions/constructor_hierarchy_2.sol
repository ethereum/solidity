pragma experimental SMTChecker;
contract C { constructor(uint) public {} }
contract A is C { constructor() C(2) public {} }
contract B is C { constructor() C(3) public {} }
contract J is C { constructor() C(3) public {} }
// ----
// Warning: (45-72): Assertion checker does not yet support constructors.
// Warning: (93-121): Assertion checker does not yet support constructors.
// Warning: (142-170): Assertion checker does not yet support constructors.
// Warning: (191-219): Assertion checker does not yet support constructors.
