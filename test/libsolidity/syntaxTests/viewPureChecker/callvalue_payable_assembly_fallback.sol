contract C
{
    receive () external payable {
    }
    fallback () external payable {
        uint x;
        assembly {
            x := callvalue()
        }
    }
}
