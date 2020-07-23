contract A { event X(uint); }
contract B is A {}
contract C is A {}
contract D is B, C {}
