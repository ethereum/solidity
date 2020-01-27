interface SuperA {
    function test1() external returns (uint256);
    function test2() external returns (uint256);
    function test3() external returns (uint256);
    function test4() external returns (uint256);
    function test5() external returns (uint256);
}

interface SuperB {
    function test1() external returns (uint256);
    function test2() external returns (uint256);
    function test3() external returns (uint256);
    function test4() external returns (uint256);
    function test5() external returns (uint256);
}

interface Sub is SuperA, SuperB {
    function test1() external returns (uint256);
    function test2() external override returns (uint256);
    function test3() external override(SuperA) returns (uint256);
    function test4() external override(SuperB) returns (uint256);
    function test5() external override(SuperA, SuperB) returns (uint256);
}

// ----
// TypeError: (572-616): Overriding function is missing "override" specifier.
// TypeError: (572-616): Overriding function is missing "override" specifier.
// TypeError: (572-616): Function needs to specify overridden contracts "SuperA" and "SuperB".
// TypeError: (647-655): Function needs to specify overridden contracts "SuperA" and "SuperB".
// TypeError: (705-721): Function needs to specify overridden contract "SuperB".
// TypeError: (771-787): Function needs to specify overridden contract "SuperA".
