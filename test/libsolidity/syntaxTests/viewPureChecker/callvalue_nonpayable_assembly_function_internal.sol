contract C
{
    function f() internal returns (uint x) {
        assembly {
            x := callvalue()
        }
    }
	function g() public returns (uint) {
		return f();
	}
}
// ----
// Warning: (17-121): Function state mutability can be restricted to view
