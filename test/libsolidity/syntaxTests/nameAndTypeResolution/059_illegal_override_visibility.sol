contract B { function f() internal {} }
contract C is B { function f() public {} }
// ----
// TypeError: (58-80): Overriding function visibility differs.
