contract BaseBase {
    uint256 m_a;

    constructor(uint256 a) public {
        m_a = a;
    }
}


contract Base is BaseBase(7) {
    constructor() public {
        m_a *= m_a;
    }
}


contract Derived is Base {
    function getA() public returns (uint256 r) {
        return m_a;
    }
}

// ====
// compileViaYul: also
// ----
// getA() -> 49
