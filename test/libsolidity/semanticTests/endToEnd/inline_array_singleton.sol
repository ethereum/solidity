contract C {
    function f() public returns(uint) {
        return [4][0];
    }
}

// ----
// f() -> 4
// f():"" -> "4"
