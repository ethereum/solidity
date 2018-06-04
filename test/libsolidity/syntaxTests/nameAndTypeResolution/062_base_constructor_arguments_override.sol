contract A { function A(uint a) public { } }
contract B is A { }
// ----
// Warning: (13-42): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (24-30): Unused function parameter. Remove or comment out the variable name to silence this warning.
