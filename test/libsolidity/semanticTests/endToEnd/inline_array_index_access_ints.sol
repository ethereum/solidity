contract C {
    function f() public returns(uint) {
        return ([1, 2, 3, 4][2]);
    }
}

// ----
// f() -> 3
