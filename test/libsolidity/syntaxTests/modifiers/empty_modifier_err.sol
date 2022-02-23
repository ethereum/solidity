contract A {modifier m virtual;}

abstract contract B {modifier m virtual;}
contract C is B { }

abstract contract D {modifier m;}
// ----
// TypeError 3656: (0-32): Contract "A" should be marked as abstract.
// TypeError 3656: (76-95): Contract "C" should be marked as abstract.
// TypeError 8063: (118-129): Modifiers without implementation must be marked virtual.
