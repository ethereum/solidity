contract Base
{
    error CustomError(uint256, string, uint256);
}

contract C is Base
{
    function f() external pure
    {
        require(false, CustomError(1, "two", 3));
    }
}
// ====
// revertStrings: strip
// targetContract: C
// ----
// f() -> FAILURE, hex"11a1077e", 1, 0x60, 3, 3, "two"
