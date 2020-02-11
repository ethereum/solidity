contract C {
    function f() public returns(uint) {
        uint[][] memory a = new uint[][](0);
        return 7;
    }
}

// ----
// f() -> 7
