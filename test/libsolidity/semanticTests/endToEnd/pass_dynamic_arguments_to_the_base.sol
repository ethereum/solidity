contract Base {
    constructor(uint i) public {
        m_i = i;
    }
    uint public m_i;
}
contract Derived is Base {
    constructor(uint i) Base(i) public {}
}
contract Final is Derived(4) {}

// ----
// m_i() -> 4
