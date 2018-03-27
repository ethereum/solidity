pragma experimental "v0.5.0";
contract test {
    function f() pure public {
        { uint x; }
        uint x;
    }
}
// ----
// Warning: Unused local variable.
// Warning: Unused local variable.
