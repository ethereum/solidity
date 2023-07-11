pragma abicoder               v2;


contract C {
    function f(bool _b) public returns (uint256) {
        if (_b) return 1;
        else return 0;
    }

    function g(bool _in) public returns (bool _out) {
        _out = _in;
    }
}
// ----
// f(bool): 0x0 -> 0x0
// f(bool): 0x1 -> 0x1
// f(bool): 0x2 -> FAILURE
// f(bool): 0x3 -> FAILURE
// f(bool): 0xff -> FAILURE
// g(bool): 0x0 -> 0x0
// g(bool): 0x1 -> 0x1
// g(bool): 0x2 -> FAILURE
// g(bool): 0x3 -> FAILURE
// g(bool): 0xff -> FAILURE
