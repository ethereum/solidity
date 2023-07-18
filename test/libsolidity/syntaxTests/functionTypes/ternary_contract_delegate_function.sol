library L {
    function f() external {}
}

contract C {
    function test() public {
        (true ? L.f : L.f).selector;
    }
}

// ----
// TypeError 9582: (94-121): Member "selector" not found or not visible after argument-dependent lookup in function ().
