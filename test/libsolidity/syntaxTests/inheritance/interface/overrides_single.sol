interface Super {
    function test1() external returns (uint256);
    function test2() external returns (uint256);
    function test3() external returns (uint256);
}

interface Sub is Super {
    function test1() external returns (uint256);
    function test2() external override returns (uint256);
    function test3() external override(Super) returns (uint256);
}

// ----
// TypeError: (197-241): Overriding function is missing "override" specifier.
