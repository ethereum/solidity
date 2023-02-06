abstract contract D {
    function d() public virtual ;
}

contract C {
    bool a;
    uint x;
    D d;
    function g() public returns (uint) {
        x = 2;
        d.d();
        return x;
    }
    function f(bool b) public {
        x = 1;
        uint y = b ? g() : 3;
        assert(x == 2 || x == 1);
    }
    function h() internal {
        x = 3;
    }
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 2072: (255-261): Unused local variable.
// Info 1180: Reentrancy property(ies) for :C:\n((!(x <= 2) || !(x' >= 3)) && (<errorCode> <= 0) && (!(x' <= 0) || !(x >= 2)))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(x == 2 || x == 1)\n
