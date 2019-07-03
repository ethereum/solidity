// test for issue #1126 https://github.com/ethereum/cpp-ethereum/issues/1126
contract Parent {
    uint256 public m_aMember;
}
contract Child is Parent {
    function foo() public returns (uint256) { return Parent.m_aMember; }
}
// ----
// Warning: (158-226): Function state mutability can be restricted to view
