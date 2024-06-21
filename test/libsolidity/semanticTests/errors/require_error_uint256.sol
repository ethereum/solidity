error CustomError(uint256);

contract C
{
    function f() external pure
    {
        require(false, CustomError(1));
    }

    function g() external pure
    {
        require(false, CustomError(2));
    }
}

// ----
// f() -> FAILURE, hex"110b3655", 1
// g() -> FAILURE, hex"110b3655", 2
