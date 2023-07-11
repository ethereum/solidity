abstract contract A {
    function f() public mod returns (bool r) {
        return true;
    }

    modifier mod virtual;
}


contract C is A {
    modifier mod override {
        if (false) _;
    }
}
// ----
// f() -> false
