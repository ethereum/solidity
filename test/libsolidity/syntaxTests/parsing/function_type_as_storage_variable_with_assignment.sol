contract test {
    function f(uint x, uint y) public returns (uint a) {}
    function (uint, uint) internal returns (uint) f1 = f;
}
// ----
