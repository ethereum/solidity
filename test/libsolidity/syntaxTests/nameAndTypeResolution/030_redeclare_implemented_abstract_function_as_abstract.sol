abstract contract base { function foo() public virtual; }
contract derived is base { function foo() public virtual override {} }
contract wrong is derived { function foo() public virtual override; }
// ----
// TypeError 4593: (157-196): Overriding an implemented function with an unimplemented function is not allowed.
