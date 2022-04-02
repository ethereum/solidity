contract A {modifier m virtual;}

abstract contract B {modifier m virtual;}
contract C is B { }

abstract contract D {modifier m;}
// ----
// TypeError 3656: (0-32='contract A {modifier m virtual;}'): Contract "A" should be marked as abstract.
// TypeError 3656: (76-95='contract C is B { }'): Contract "C" should be marked as abstract.
// TypeError 8063: (118-129='modifier m;'): Modifiers without implementation must be marked virtual.
