contract C {
	uint8 x;

	function inc_pre() public {
		++x;
	}

	function dec_pre() public {
		--x;
	}

    function inc_post() public {
		x++;
	}

	function dec_post() public {
		x--;
	}

}
// ====
// SMTEngine: bmc
// ----
// Warning 2661: (55-58): BMC: Overflow (resulting value larger than 255) happens here.
// Warning 4144: (95-98): BMC: Underflow (resulting value less than 0) happens here.
// Warning 2661: (139-142): BMC: Overflow (resulting value larger than 255) happens here.
// Warning 4144: (180-183): BMC: Underflow (resulting value less than 0) happens here.
