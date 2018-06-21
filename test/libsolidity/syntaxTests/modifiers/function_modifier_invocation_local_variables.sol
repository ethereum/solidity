contract B {
    function f() mod(x) pure public { uint x = 7; }
    modifier mod(uint a) { if (a > 0) _; }
}
// ----
// DeclarationError: (34-35): Undeclared identifier.
