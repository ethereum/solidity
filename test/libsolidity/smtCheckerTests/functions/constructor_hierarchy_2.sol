pragma experimental SMTChecker;
contract C { uint a; constructor(uint x) { a = x; } }
contract A is C { constructor() C(2) { assert(a == 2); } }
contract B is C { constructor() C(3) { assert(a == 3); } }
contract J is C { constructor() C(3) { assert(a == 4); } }
// ----
// Warning 6328: (243-257): Assertion violation happens here
