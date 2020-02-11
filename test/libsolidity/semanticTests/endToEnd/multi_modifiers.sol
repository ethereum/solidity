contract C {
    uint public x;
    modifier m1 {
        address a1 = msg.sender;
        x++;
        _;
    }

    function f1() m1() public {
        x += 7;
    }

    function f2() m1() public {
        x += 3;
    }
}

// ----
// f1() -> bytes(
// f1():"" -> ""
// x() -> 8
// x():"" -> "8"
// f2() -> bytes(
// f2():"" -> ""
// x() -> 12
// x():"" -> "12"
