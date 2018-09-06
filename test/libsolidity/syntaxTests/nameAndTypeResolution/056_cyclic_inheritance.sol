contract A is B { }
contract B is A { }
// ----
// TypeError: (14-15): Definition of base has to precede definition of derived contract
