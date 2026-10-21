uint256 constant before = 1;

contract C {
    uint256 constant before = 2; // Shadows the file-level constant
    uint256 constant later = 3; // Shadows the file-level constant despite the order

    function f() public pure returns (uint256) {
        uint256 localLater = 4; // Shadows the file-level constant despite the order
        return localLater;
    }
}

function free() pure returns (uint256) {
    { uint256 freeLater = 5; } // Shadows the file-level constant, not the later local
    uint256 freeLater = 6; // Shadows the file-level constant despite the order
    return freeLater;
}

uint256 constant later = 7;
uint256 constant localLater = 8;
uint256 constant freeLater = 9;
// ----
// Warning 2519: (254-272): This declaration shadows an existing declaration.
// Warning 2519: (47-74): This declaration shadows an existing declaration.
// Warning 2519: (115-141): This declaration shadows an existing declaration.
// Warning 2519: (414-431): This declaration shadows an existing declaration.
// Warning 2519: (499-516): This declaration shadows an existing declaration.
// Warning 2072: (414-431): Unused local variable.
