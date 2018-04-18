pragma experimental "v0.5.0";
contract A { function A() public {} }
// ----
// SyntaxError: (43-65): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
