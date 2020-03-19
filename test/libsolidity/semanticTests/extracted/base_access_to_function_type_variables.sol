contract C {
    function() returns (uint256) internal x;

    function set() public {
        C.x = g;
    }

    function g() public pure returns (uint256) {
        return 2;
    }

    function h() public returns (uint256) {
        return C.x();
    }
}

// ----
// g() -> 2
// h() -> FAILURE
// set() ->
// h() -> 2
