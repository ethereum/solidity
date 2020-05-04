contract Base {
    constructor() public {}

    uint256 m_base = 5;

    function getBMember() public returns (uint256 i) {
        return m_base;
    }
}


contract Derived is Base {
    constructor() public {}

    uint256 m_derived = 6;

    function getDMember() public returns (uint256 i) {
        return m_derived;
    }
}
// ====
// compileViaYul: also
// ----
// getBMember() -> 5
// getDMember() -> 6
