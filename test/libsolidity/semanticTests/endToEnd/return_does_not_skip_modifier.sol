contract C {
    uint public x;
    modifier setsx {
        _;
        x = 9;
    }

    function f() setsx public returns(uint) {
        return 2;
    }
}

// ----
// x() -> 0
// x():"" -> "0"
// f() -> 2
// f():"" -> "2"
// x() -> 9
// x():"" -> "9"
