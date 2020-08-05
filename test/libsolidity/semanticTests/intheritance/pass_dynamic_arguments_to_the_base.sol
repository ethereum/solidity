contract Base {
    constructor(uint256 i) {
        m_i = i;
    }

    uint256 public m_i;
}


contract Derived is Base {
    constructor(uint256 i) Base(i) {}
}


contract Final is Derived(4) {}

// ====
// compileViaYul: also
// ----
// m_i() -> 4
