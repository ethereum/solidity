contract Base {
    constructor(uint j) public {
        m_i = j;
    }
    uint public m_i;
}
contract Base1 is Base {
    constructor(uint k) Base(k) public {}
}
contract Derived is Base, Base1 {
    constructor(uint i) Base1(i) public {}
}
contract Final is Derived(4) {}

// ----
// m_i() -> 4
