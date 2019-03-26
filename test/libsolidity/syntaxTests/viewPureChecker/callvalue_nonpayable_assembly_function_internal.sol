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
