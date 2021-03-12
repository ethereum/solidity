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
// m_x() -> 7
// m_s() -> 0x20, 0x4e, 0x6162636465666768696a6b6c6d6e6f707172737475767778797a616263646566, 0x6768696a6b6c6d6e6f707172737475767778797a6162636465666768696a6b6c, 49497222798007550449728244510595746054660801178848437687526699964558959706112
