pragma experimental SMTChecker;
contract C { uint a; constructor(uint x) public { a = x; } }
contract A is C { constructor() C(2) public { assert(a == 2); } }
contract B is C { constructor() C(3) public { assert(a == 3); } }
contract J is C { constructor() C(3) public { assert(a == 4); } }
// ----
// Warning: (271-285): Assertion violation happens here
