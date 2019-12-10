contract A {
    uint[] x;
    function g() public returns (uint) {
        return x.push();
    }
}
// ----