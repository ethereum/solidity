contract Parent1 {
    uint256 internal m_aMember1;
}
contract Parent2 is Parent1 {
    uint256 internal m_aMember2;
}
contract Child is Parent2 {
    function foo() public returns (uint256) { return Child.m_aMember2; }
    uint256 public m_aMember3;
}
// ----
// TypeError: (200-216): Member "m_aMember2" not found or not visible after argument-dependent lookup in type(contract Child).
