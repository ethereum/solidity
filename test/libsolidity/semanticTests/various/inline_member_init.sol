contract test {
    constructor() {
        m_b = 6;
        m_c = 8;
    }

    uint256 m_a = 5;
    uint256 m_b;
    uint256 m_c = 7;

    function get() public returns (uint256 a, uint256 b, uint256 c) {
        a = m_a;
        b = m_b;
        c = m_c;
    }
}
// ----
// get() -> 5, 6, 8
