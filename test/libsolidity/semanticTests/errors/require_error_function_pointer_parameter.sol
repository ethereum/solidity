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

// ----
// f() -> FAILURE, hex"271b1dfa", hex"c06afe3a8444fc0004668591e8306bfb9968e79ef37cdc8e0000000000000000"
