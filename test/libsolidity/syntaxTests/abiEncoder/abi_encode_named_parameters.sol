contract C {
    function f() public pure returns (bytes memory) {
        return abi.encode({x: uint256(1), y: address(0)});
    }
}

// ----
// TypeError 2627: (82-124): Named arguments cannot be used for functions that take arbitrary parameters.
