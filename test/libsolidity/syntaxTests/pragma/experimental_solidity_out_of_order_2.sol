function f() pure returns (uint)
{
    return 1;
}

pragma experimental solidity;

struct A
{
    uint256 x;
}
// ====
// EVMVersion: >=constantinople
// ----
// ParserError 8185: (83-89): Experimental pragma "solidity" can only be set at the beginning of the source unit.
