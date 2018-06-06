contract C {
    struct S { uint a; }
    S s;
    function f() view public {
        S storage x = s;
        x;
    }
    function g() view public {
        S storage x = s;
        x = s;
    }
    function i() public {
        s.a = 2;
    }
    function h() public {
        S storage x = s;
        x.a = 2;
    }
}
