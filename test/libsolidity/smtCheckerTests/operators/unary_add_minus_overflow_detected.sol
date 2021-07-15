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
// SMTEngine: all
// ----
// Warning 4984: (55-58): CHC: Overflow (resulting value larger than 255) might happen here.
// Warning 3944: (95-98): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.dec_pre()
// Warning 4984: (136-139): CHC: Overflow (resulting value larger than 255) might happen here.
// Warning 3944: (180-183): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.dec_post()
// Warning 2661: (55-58): BMC: Overflow (resulting value larger than 255) happens here.
// Warning 2661: (136-139): BMC: Overflow (resulting value larger than 255) happens here.
