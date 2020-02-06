contract C {
    uint256 x;
    uint256 y;
    function set(uint256 v) public returns (uint256) { x = v; return v; }
    function f() public returns (uint256, uint256) {
       (y, y, y) = (set(1), set(2), set(3));
       assert(y == 1 && x == 3);
       return (x, y);
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 3, 1
