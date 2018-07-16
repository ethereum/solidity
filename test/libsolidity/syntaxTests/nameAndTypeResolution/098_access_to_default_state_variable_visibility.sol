contract c {
    uint a;
}
contract d {
    function g() public { c(0).a(); }
}
// ----
// TypeError: (66-72): Member "a" not found or not visible after argument-dependent lookup in contract c.
