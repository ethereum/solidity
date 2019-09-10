library L1 {
    using L1 for *;
    function f() public pure returns (uint r) { return r.g(); }
    function g(uint) public pure returns (uint) { return 2; }
}

library L2 {
    using L1 for *;
    function f() public pure returns (uint r) { return r.g(); }
    function g(uint) public pure returns (uint) { return 2; }
}
// ----
// TypeError: (88-93): Libraries cannot call their own functions externally.
