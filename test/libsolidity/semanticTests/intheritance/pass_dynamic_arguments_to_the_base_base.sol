contract Base {
    constructor(uint256 j) public {
        m_i = j;
    }

    uint256 public m_i;
}


contract Base1 is Base {
    constructor(uint256 k) public Base(k) {}
}


contract Derived is Base, Base1 {
    constructor(uint256 i) public Base1(i) {}
}


contract Final is Derived(4) {}

// ====
// compileViaYul: also
// ----
// m_i() -> 4
