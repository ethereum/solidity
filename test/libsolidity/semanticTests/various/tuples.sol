contract C {
    uint256[] data;
    uint256[] m_c;

    function g() internal returns (uint256 a, uint256 b, uint256[] storage c) {
        return (1, 2, data);
    }

    function h() external returns (uint256 a, uint256 b) {
        return (5, 6);
    }

    function f() public returns (uint256) {
        data.push(3);
        uint256 a;
        uint256 b;
        (a, b) = this.h();
        if (a != 5 || b != 6) return 1;
        uint256[] storage c = m_c;
        (a, b, c) = g();
        if (a != 1 || b != 2 || c[0] != 3) return 2;
        (a, b) = (b, a);
        if (a != 2 || b != 1) return 3;
        (a, , b, , ) = (8, 9, 10, 11, 12);
        if (a != 8 || b != 10) return 4;
    }
}
// ----
// f() -> 0
