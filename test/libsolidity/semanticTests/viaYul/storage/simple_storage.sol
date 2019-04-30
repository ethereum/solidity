contract C {
    uint x;
    uint y;
    function setX(uint a) public returns (uint _x) {
        x = a;
        _x = x;
    }
    function setY(uint a) public returns (uint _y) {
        y = a;
        _y = y;
    }
}
// ====
// compileViaYul: true
// ----
// setX(uint256): 6 -> 6
// setY(uint256): 2 -> 2
