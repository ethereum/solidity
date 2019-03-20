contract C
{
	modifier m {
		uint x;
		assembly {
			x := callvalue()
		}
		_;
	}
    function f() m public payable {
    }
}
