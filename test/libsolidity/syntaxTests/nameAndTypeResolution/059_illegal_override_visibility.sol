contract B { function f() virtual internal {} }
contract C is B { function f() public {} }
// ----
// TypeError 9456: (66-88='function f() public {}'): Overriding function is missing "override" specifier.
// TypeError 9098: (66-88='function f() public {}'): Overriding function visibility differs.
