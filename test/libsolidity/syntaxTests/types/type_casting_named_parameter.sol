contract C {
    function f() public pure returns (uint256) {
        int256 x = -1;
        return uint256({value: x});
    }
}

// ----
// TypeError 5153: (100-119): Type conversion cannot allow named arguments.
