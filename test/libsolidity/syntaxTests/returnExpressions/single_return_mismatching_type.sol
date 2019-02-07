contract C
{
    function g() public pure returns (uint)
    {
        return "string";
    }
}
// ----
// TypeError: (78-86): Return argument type literal_string "string" is not implicitly convertible to expected type (type of first return variable) uint256.
