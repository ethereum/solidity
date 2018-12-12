contract C {
    uint[] s;
    modifier mod(uint[] storage b) {
        b[0] = 0;
        _;
    }
    function f() mod(a) internal returns (uint[] storage a)
    {
		a = s;
    }
}
// ----
// TypeError: (120-121): This variable is of storage pointer type and can be accessed without prior assignment.
