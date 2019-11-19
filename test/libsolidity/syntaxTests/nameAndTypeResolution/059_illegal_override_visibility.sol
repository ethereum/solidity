contract B { function f() virtual internal {} }
contract C is B { function f() public {} }
// ----
// TypeError: (66-88): Overriding function is missing 'override' specifier.
// TypeError: (66-88): Overriding function visibility differs.
