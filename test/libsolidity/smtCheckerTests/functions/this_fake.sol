pragma experimental SMTChecker;

contract C
{
    uint public x;
    C c;
    function f(C _c) public {
        c = _c;
    }
    function g() public {
        C this = c;
		x = 0;
        this.h();
		// State knowledge is erased.
		// Function call is not inlined.
		assert(x == 0);
    }
    function h() public {
        x = 2;
    }
}
// ----
// Warning: (160-166): This declaration shadows a builtin symbol.
// Warning: (268-282): Assertion violation happens here
