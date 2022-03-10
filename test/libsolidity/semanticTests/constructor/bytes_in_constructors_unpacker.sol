contract Test {
    uint public m_x;
    bytes public m_s;
    constructor(uint x, bytes memory s) {
        m_x = x;
        m_s = s;
    }
}
// ====
// compileViaYul: also
// ----
// constructor(): 7, 0x40, 78, "abcdefghijklmnopqrstuvwxyzabcdef", "ghijklmnopqrstuvwxyzabcdefghijkl", "mnopqrstuvwxyz" ->
// gas irOptimized: 291443
// gas legacy: 309607
// gas legacyOptimized: 260566
// m_x() -> 7
// m_s() -> 0x20, 78, "abcdefghijklmnopqrstuvwxyzabcdef", "ghijklmnopqrstuvwxyzabcdefghijkl", "mnopqrstuvwxyz"
