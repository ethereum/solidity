pragma experimental SMTChecker;
contract C {
	uint8 x;

	function inc_pre() public {
		++x;
	}

	function dec_pre() public {
		--x;
	}

	/* Commented out because Spacer segfaults in Z3 4.8.9
    function inc_post() public {
		x++;
	}

	function dec_post() public {
		x--;
	}
    */

}
// ====
// SMTEngine: bmc
// ----
// Warning 2661: (87-90): BMC: Overflow (resulting value larger than 255) happens here.
// Warning 4144: (127-130): BMC: Underflow (resulting value less than 0) happens here.
