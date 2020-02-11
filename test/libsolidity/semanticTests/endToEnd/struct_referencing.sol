pragma experimental ABIEncoderV2;
interface I {
    struct S {
        uint a;
    }
}
library L {
    struct S {
        uint b;
        uint a;
    }

    function f() public pure returns(S memory) {
        S memory s;
        s.a = 3;
        return s;
    }

    function g() public pure returns(I.S memory) {
        I.S memory s;
        s.a = 4;
        return s;
    }
    // argument-dependant lookup tests
    function a(I.S memory) public pure returns(uint) {
        return 1;
    }

    function a(S memory) public pure returns(uint) {
        return 2;
    }
}
contract C is I {
    function f() public pure returns(S memory) {
        S memory s;
        s.a = 1;
        return s;
    }

    function g() public pure returns(I.S memory) {
        I.S memory s;
        s.a = 2;
        return s;
    }

    function h() public pure returns(L.S memory) {
        L.S memory s;
        s.a = 5;
        return s;
    }

    function x() public pure returns(L.S memory) {
        return L.f();
    }

    function y() public pure returns(I.S memory) {
        return L.g();
    }

    function a1() public pure returns(uint) {
        S memory s;
        return L.a(s);
    }

    function a2() public pure returns(uint) {
        L.S memory s;
        return L.a(s);
    }
}

// ----
// f() -> 0, 3
// f():"" -> "0, 3"
// g() -> 4
// g():"" -> "4"

pragma experimental ABIEncoderV2;
interface I {
    struct S {
        uint a;
    }
}
library L {
    struct S {
        uint b;
        uint a;
    }

    function f() public pure returns(S memory) {
        S memory s;
        s.a = 3;
        return s;
    }

    function g() public pure returns(I.S memory) {
        I.S memory s;
        s.a = 4;
        return s;
    }
    // argument-dependant lookup tests
    function a(I.S memory) public pure returns(uint) {
        return 1;
    }

    function a(S memory) public pure returns(uint) {
        return 2;
    }
}
contract C is I {
    function f() public pure returns(S memory) {
        S memory s;
        s.a = 1;
        return s;
    }

    function g() public pure returns(I.S memory) {
        I.S memory s;
        s.a = 2;
        return s;
    }

    function h() public pure returns(L.S memory) {
        L.S memory s;
        s.a = 5;
        return s;
    }

    function x() public pure returns(L.S memory) {
        return L.f();
    }

    function y() public pure returns(I.S memory) {
        return L.g();
    }

    function a1() public pure returns(uint) {
        S memory s;
        return L.a(s);
    }

    function a2() public pure returns(uint) {
        L.S memory s;
        return L.a(s);
    }
}

// ----
// f() -> 1
// f():"" -> "1"
// g() -> 2
// g():"" -> "2"
// h() -> 0, 5
// h():"" -> "0, 5"
// x() -> 0, 3
// x():"" -> "0, 3"
// y() -> 4
// y():"" -> "4"
// a1() -> 1
// a1():"" -> "1"
// a2() -> 2
// a2():"" -> "2"
