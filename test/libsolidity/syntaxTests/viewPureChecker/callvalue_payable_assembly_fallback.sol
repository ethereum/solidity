contract C
{
    function () external payable {
        uint x;
        assembly {
            x := callvalue()
        }
    }
}
