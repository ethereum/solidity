contract base { function foo(); }
contract derived is base { function foo() public {} }
contract wrong is derived { function foo(); }
// ----
// TypeError: (116-131): Redeclaring an already implemented function as abstract
