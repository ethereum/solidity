contract A { constructor(uint a) public { } }
contract B is A { }
// ----
// Warning: (25-31): Unused function parameter. Remove or comment out the variable name to silence this warning.
