error Error(uint256);

contract C
{
    error OtherError(uint256);

    function f() external pure
    {
        revert Error(require(false, OtherError(1)));
    }
}
// ----
// TypeError 9553: (126-155): Invalid type for argument in function call. Invalid implicit conversion from tuple() to uint256 requested.
