contract C {
    uint[] s;
    modifier mod(uint[] storage b) {
        _;
        b[0] = 0;
    }
    function f() mod(a) internal returns (uint[] storage a)
    {
		a = s;
    }
}
// ----
// TypeError: (120-121): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
