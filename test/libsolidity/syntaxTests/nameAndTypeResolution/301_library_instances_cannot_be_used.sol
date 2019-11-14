library L { function l() public {} }
contract test {
    function f() public {
        L x;
        x.l();
    }
}
// ----
// TypeError: (87-90): The type of a variable cannot be a library.
// TypeError: (100-103): Member "l" not found or not visible after argument-dependent lookup in library L.
