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

// ====
// compileViaYul: true
// ----
// f() -> FAILURE, hex"110b3655", hex"0000000000000000000000000000000000000000000000000000000000000001"
// g() -> FAILURE, hex"110b3655", hex"0000000000000000000000000000000000000000000000000000000000000002"
