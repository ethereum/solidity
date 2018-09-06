contract Parent {
    uint256 internal m_aMember;
}
contract Child is Parent {
    function foo() public returns (uint256) { return Parent.m_aMember; }
}
// ----
// Warning: (83-151): Function state mutability can be restricted to pure
