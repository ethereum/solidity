library L {
    function x() internal returns (uint256) {
        return 7;
    }
}


contract C {
    function() returns (uint256) internal x;

    constructor() public {
        x = L.x;
    }

    function t() public returns (uint256) {
        return x();
    }
}

// ----
// t() -> 7
