contract C {
    uint immutable x = f();

    function f() public pure returns (uint) { return 3; }
}
