contract C
{
	modifier m {
		uint x;
		assembly {
			x := callvalue()
		}
		_;
	}
    function f() m public {
    }
}
// ----
// TypeError: (99-100): This modifier uses "msg.value" or "callvalue()" and thus the function has to be payable or internal.
