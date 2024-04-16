contract C
{
    function f() public pure returns (bool) {
        bool blobhash = true;
        return blobhash;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// Warning 2319: (67-80): This declaration shadows a builtin symbol.
