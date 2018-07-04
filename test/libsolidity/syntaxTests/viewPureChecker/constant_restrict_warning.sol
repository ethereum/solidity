contract C {
    uint constant x = 2;
    function f() view public returns (uint) {
        return x;
    }
    function g() public returns (uint) {
        return x;
    }
}
// ----
// Warning: (42-107): Function state mutability can be restricted to pure
// Warning: (112-172): Function state mutability can be restricted to pure
