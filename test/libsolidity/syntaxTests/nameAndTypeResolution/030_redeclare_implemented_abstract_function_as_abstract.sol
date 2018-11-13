contract base { function foo() public; }
contract derived is base { function foo() public {} }
contract wrong is derived { function foo() public; }
// ----
// TypeError: (123-145): Redeclaring an already implemented function as abstract
