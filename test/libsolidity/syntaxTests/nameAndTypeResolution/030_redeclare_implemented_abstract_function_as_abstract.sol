abstract contract base { function foo() public; }
contract derived is base { function foo() public override {} }
contract wrong is derived { function foo() public; }
// ----
// TypeError: (141-163): Overriding function is missing 'override' specifier.
// TypeError: (141-163): Redeclaring an already implemented function as abstract
