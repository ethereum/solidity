contract C {
    struct S {
        uint256 x;
    }
    S s;

    function g() internal returns (uint256, S storage, uint256) {
        s.x = 7;
        return (1, s, 2);
    }

    function f() public returns (bool) {
        (uint256 x1, S storage y1, uint256 z1) = g();
        if (x1 != 1 || y1.x != 7 || z1 != 2) return false;
        (, S storage y2, ) = g();
        if (y2.x != 7) return false;
        (uint256 x2, , ) = g();
        if (x2 != 1) return false;
        (, , uint256 z2) = g();
        if (z2 != 2) return false;
        return true;
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> true
