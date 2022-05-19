contract BaseBase {
    uint256 m_a;

    constructor(uint256 a) {
        m_a = a;
    }
}


contract Base is BaseBase(7) {
    constructor() {
        m_a *= m_a;
    }
}


contract Derived is Base {
    function getA() public returns (uint256 r) {
        return m_a;
    }
}

// ----
// getA() -> 49
