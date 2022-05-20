contract BaseBase {
    uint256 m_a;

    constructor(uint256 a) {
        m_a = a;
    }

    function g() public returns (uint256 r) {
        return 2;
    }
}


contract Base is BaseBase(BaseBase.g()) {}


contract Derived is Base {
    function getA() public returns (uint256 r) {
        return m_a;
    }
}

// ----
// getA() -> 2
