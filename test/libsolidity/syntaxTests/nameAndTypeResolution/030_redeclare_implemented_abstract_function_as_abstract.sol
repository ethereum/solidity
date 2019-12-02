abstract contract base { function foo() public virtual; }
contract derived is base { function foo() public virtual override {} }
contract wrong is derived { function foo() public virtual override; }
// ----
// TypeError: (157-196): Redeclaring an already implemented function as abstract
