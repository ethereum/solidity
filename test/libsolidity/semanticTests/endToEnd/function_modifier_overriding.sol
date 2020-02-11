contract A {
    function f() mod public returns(bool r) {
        return true;
    }
    modifier mod virtual {
        _;
    }
}
contract C is A {
    modifier mod override {
        if (false) _;
    }
}

// ----
// f() -> false
