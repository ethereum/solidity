contract C {
    struct S { uint x; }
    S s;
    function f() view internal returns (S storage) {
        return s;
    }
    function g() public {
        f().x = 2;
    }
    function h() view public {
        f();
        f().x;
    }
}
