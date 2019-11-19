abstract contract base { function foo() public virtual; }
contract derived is base { function foo() public virtual override {} }
contract wrong is derived { function foo() public; }
// ----
// TypeError: (157-179): Overriding function is missing 'override' specifier.
// TypeError: (157-179): Redeclaring an already implemented function as abstract
