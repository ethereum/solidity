contract test {
    function f() public {
        uint uint10abc = 3;
        int int10abc = 4;
        uint10abc; int10abc;
    }
}
// ----
// Warning: (20-130): Function state mutability can be restricted to pure
