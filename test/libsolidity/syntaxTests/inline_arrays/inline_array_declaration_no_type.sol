contract C {
    function f() public returns (uint) {
        return ([4,5,6][1]);
    }
}
// ----
// Warning: (17-88): Function state mutability can be restricted to pure
