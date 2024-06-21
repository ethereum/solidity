error CustomError(uint256);

contract C
{
    function f() public pure returns (uint256)
    {
        require(false, require(CustomError(1)));
        return 2;
    }
}

// ----
// TypeError 9322: (118-125): No matching declaration found after argument-dependent lookup.
