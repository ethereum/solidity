error CustomError(function(uint256) external pure returns (uint256));

contract C
{
    function e(uint256 x) external pure returns (uint256)
    {
        return x;
    }

    function f() external view
    {
        // more than one stack slot
        require(false, CustomError(this.e));
    }
}

// ====
// compileToEOF: true
// EVMVersion: >=prague
// ----
// f() -> FAILURE, hex"271b1dfa", hex"dfc163ea0fefc2097b7425134f69fcafa3742b0af37cdc8e0000000000000000"
