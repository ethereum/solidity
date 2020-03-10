contract C {
    uint256 immutable x;
    uint256 immutable y;
    constructor() public {
        x = 42;
        y = x;
    }
    function f() public view returns (uint256, uint256) {
        return (x+x,y);
    }
}
// ----
// f() -> 84, 42
