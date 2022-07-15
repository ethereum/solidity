contract C {
    function f(uint256[] memory x) private {}
    function f(uint256[3] memory x) private {}

    function g() private {
        f([1]);
        f([1,2,3]);
        f([1,2,3,4]);
    }
}

// ----
// TypeError 4487: (158-159): No unique declaration found after argument-dependent lookup.
