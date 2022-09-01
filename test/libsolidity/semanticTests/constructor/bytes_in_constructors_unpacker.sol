contract Test {
    uint public m_x;
    bytes public m_s;
    constructor(uint x, bytes memory s) {
        m_x = x;
        m_s = s;
    }
}
// ----
// constructor(): 7, 0x40, 78, "abcdefghijklmnopqrstuvwxyzabcdef", "ghijklmnopqrstuvwxyzabcdefghijkl", "mnopqrstuvwxyz" ->
// gas irOptimized: 270118
// gas legacy: 315616
// gas legacyOptimized: 260686
// m_x() -> 7
// m_s() -> 0x20, 78, "abcdefghijklmnopqrstuvwxyzabcdef", "ghijklmnopqrstuvwxyzabcdefghijkl", "mnopqrstuvwxyz"
