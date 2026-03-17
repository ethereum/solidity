contract C {
    function add(uint x, uint y) public pure returns (uint) { return x + y; }

    function callViaFunctionPointer() public pure returns (uint) {
        function(uint, uint) pure returns (uint) fp = add;
        return fp({y: 2, x: 1});
    }
}

// ----
// TypeError 4974: (233-249): Named argument "y" does not match function declaration.
// TypeError 4974: (233-249): Named argument "x" does not match function declaration.
