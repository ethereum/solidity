// It is fine to "override" constructor of a base class since it is invisible
contract A { function A() public { } }
contract B is A { function A() public pure returns (uint8) {} }
// ----
// Warning: (91-114): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (135-178): This declaration shadows an existing declaration.
