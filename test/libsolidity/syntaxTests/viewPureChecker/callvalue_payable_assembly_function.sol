contract C
{
    function f(uint x) public payable {
        assembly {
            x := callvalue()
        }
    }
}
