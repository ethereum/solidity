contract C {
    function g() public returns (uint256 a, uint256 b, uint256 c) {
        a = 1;
        b = 2;
        c = 3;
    }

    function h() public returns (uint256 a, uint256 b, uint256 c, uint256 d) {
        a = 1;
        b = 2;
        c = 3;
        d = 4;
    }

    function f1() public returns (bool) {
        (uint256 x, uint256 y, uint256 z) = g();
        if (x != 1 || y != 2 || z != 3) return false;
        (, uint256 a, ) = g();
        if (a != 2) return false;
        (uint256 b, , ) = g();
        if (b != 1) return false;
        (, , uint256 c) = g();
        if (c != 3) return false;
        return true;
    }

    function f2() public returns (bool) {
        (uint256 a1, , uint256 a3, ) = h();
        if (a1 != 1 || a3 != 3) return false;
        (uint256 b1, uint256 b2, , ) = h();
        if (b1 != 1 || b2 != 2) return false;
        (, uint256 c2, uint256 c3, ) = h();
        if (c2 != 2 || c3 != 3) return false;
        (, , uint256 d3, uint256 d4) = h();
        if (d3 != 3 || d4 != 4) return false;
        (uint256 e1, , uint256 e3, uint256 e4) = h();
        if (e1 != 1 || e3 != 3 || e4 != 4) return false;
        return true;
    }

    function f() public returns (bool) {
        return f1() && f2();
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> true
