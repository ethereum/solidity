contract C {
    uint public x;
    modifier run() {
        for (uint i = 1; i < 10; i++) {
            if (i == 5) return;
            _;
        }
    }

    function f() run public {
        uint k = x;
        uint t = k + 1;
        x = t;
    }
}

// ----
// x() -> 0
// x():"" -> "0"
// f() -> 
// f():"" -> ""
// x() -> 4
// x():"" -> "4"
