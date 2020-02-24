contract C {
    function a() public returns (uint256) {
        return 7;
    }

    function test() public returns (uint256) {
        function() returns (uint256) y = a;
        delete y;
        y();
    }
}

// ====
// compileViaYul: also
// ----
// test() -> FAILURE
