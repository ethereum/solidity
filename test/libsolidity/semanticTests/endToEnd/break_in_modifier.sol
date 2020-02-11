contract C {
    uint public x;
    modifier run() {
        for (uint i = 0; i < 10; i++) {
            _;
            break;
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
// f() -> 
// x() -> 1
