contract base { function foo() public; }
contract derived is base { function foo() public override {} }
contract wrong is derived { function foo() public; }
// ----
// TypeError: (0-40): Contract "base" should be marked as abstract.
// TypeError: (132-154): Overriding function is missing 'override' specifier.
// TypeError: (132-154): Redeclaring an already implemented function as abstract
