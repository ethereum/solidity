pragma experimental "v0.5.0";
contract B {
    function f() mod(x) pure public { uint x = 7; }
    modifier mod(uint a) { if (a > 0) _; }
}
// ----
// DeclarationError: (64-65): Undeclared identifier.
