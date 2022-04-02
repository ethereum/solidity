contract C {
    function f() public {
        bytes memory a;
        bytes memory b = a.concat();
    }
}
// ----
// TypeError 9582: (88-96='a.concat'): Member "concat" not found or not visible after argument-dependent lookup in bytes memory.
