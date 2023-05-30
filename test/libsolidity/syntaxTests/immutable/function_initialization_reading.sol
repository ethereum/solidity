contract C {
    uint immutable x = f();

    function f() public view returns (uint) { return 3 + x; }
}
