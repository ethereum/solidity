contract Base {
    constructor(uint256 i) public {
        m_i = i;
    }

    uint256 public m_i;
}


abstract contract Base1 is Base {
    constructor(uint256 k) public {}
}


contract Derived is Base, Base1 {
    constructor(uint256 i) public Base(i) Base1(7) {}
}


contract Final is Derived(4) {}

// ====
// compileViaYul: also
// ----
// m_i() -> 4
