contract C {
    mapping(uint => uint) a;
    function f() view public {
        a;
    }
    function g() view public {
        a[2];
    }
    function h() public {
        a[2] = 3;
    }
}
