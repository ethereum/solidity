contract C
{
    function f(uint x) public {
        assembly {
            x := callvalue()
        }
    }
}
// ----
// Warning: (17-108): Function state mutability can be restricted to view
