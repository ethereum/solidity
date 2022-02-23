contract C
{
    function f() public pure {
        uint16 a = 1;
        int32 b = a;

        uint256 c = 10;
        int8 d = c;
    }
}
// ----
// TypeError 9574: (74-85): Type uint16 is not implicitly convertible to expected type int32.
// TypeError 9574: (120-130): Type uint256 is not implicitly convertible to expected type int8.
