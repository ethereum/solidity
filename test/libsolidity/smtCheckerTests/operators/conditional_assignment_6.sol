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
// ----
// Warning 2072: (255-261): Unused local variable.
// Warning 0: (59-367): Contract invariants for :C:\n((!(x' <= 0) || !(x >= 2)) && (<errorCode> <= 0) && (!(x <= 2) || !(x' >= 3)))\n((!(x' <= 0) || ((x' + ((- 1) * x)) = 0)) && (<errorCode> <= 0) && (!(x <= 2) || !(x' >= 3)))\n
