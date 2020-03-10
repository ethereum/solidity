contract C {
    uint immutable x;
    uint immutable y;
    constructor() public {
        (x, y) = f();
    }

    function f() internal pure returns(uint _x, uint _y) {
        _x = 3;
        _y = 4;
    }
}
// ----
