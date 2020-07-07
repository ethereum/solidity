contract A { function A() public {} }
// ----
// SyntaxError 5796: (13-35): Functions are not allowed to have the same name as the contract. If you intend this to be a constructor, use "constructor(...) { ... }" to define it.
// Warning 2519: (13-35): This declaration shadows an existing declaration.
