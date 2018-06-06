contract Parent1 {
    uint256 internal m_aMember1;
}
contract Parent2 is Parent1 {
    uint256 internal m_aMember2;
}
contract Child is Parent2 {
    function foo() public returns (uint256) { return Parent2.m_aMember1; }
}
// ----
// TypeError: (200-218): Member "m_aMember1" not found or not visible after argument-dependent lookup in type(contract Parent2)
