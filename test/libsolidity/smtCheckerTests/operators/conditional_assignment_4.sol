pragma experimental SMTChecker;

abstract contract D {
    function d() public virtual ;
}

contract C {
    bool a;
    uint x;
    D d;
    function g() public returns (uint) {
        x = 2;
        return x;
    }
    function f(bool b) public {
        x = 1;
        uint y = b ? g() : 3;
        assert(x == 2 || x == 1);
    }
    function h() public {
        x = 3;
    }
}
// ----
// Warning 2072: (273-279): Unused local variable.
