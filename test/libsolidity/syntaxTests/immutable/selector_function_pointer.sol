contract C {
    uint immutable x;
    constructor() public {
        x = 3;
        readX().selector;
    }

    function f() external view returns(uint)  {
        return x;
    }

    function readX() public view returns(function() external view returns(uint) _f) {
        _f = this.f;
    }
}
// ----
