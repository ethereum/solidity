contract test {
    function f() pure public returns (uint) {
        uint x = 7;
        {
            x = 3; // This should still assign to the outer variable
            uint x;
            x = 4; // This should assign to the new one
        }
        return x;
    }
    function g() pure public returns (uint x) {
        x = 7;
        {
            x = 3;
            uint x;
            return x; // This returns the new variable, i.e. 0
        }
    }
    function h() pure public returns (uint x, uint a, uint b) {
        x = 7;
        {
            x = 3;
            a = x; // This should read from the outer
            uint x = 4;
            b = x;
        }
    }
    function i() pure public returns (uint x, uint a) {
        x = 7;
        {
            x = 3;
            uint x = x; // This should read from the outer and assign to the inner
            a = x;
        }
    }
}
// ====
// compileViaYul: also
// ----
// f() -> 3
// g() -> 0
// h() -> 3, 3, 4
// i() -> 3, 3
