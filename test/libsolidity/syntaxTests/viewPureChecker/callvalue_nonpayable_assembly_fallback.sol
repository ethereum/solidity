contract C
{
    function () external {
        uint x;
        assembly {
            x := callvalue()
        }
    }
}
// ----
