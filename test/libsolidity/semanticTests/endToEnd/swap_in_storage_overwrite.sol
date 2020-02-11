contract c {
    struct S {
        uint a;
        uint b;
    }
    S public x;
    S public y;

    function set() public {
        x.a = 1;
        x.b = 2;
        y.a = 3;
        y.b = 4;
    }

    function swap() public {
        (x, y) = (y, x);
    }
}

// ----
// x() -> 0, 0
// x():"" -> "0, 0"
// y() -> 0, 0
// y():"" -> "0, 0"
// set() -> 
// set():"" -> ""
// x() -> 1, 2
// x():"" -> "1, 2"
// y() -> 3, 4
// y():"" -> "3, 4"
// swap() -> 
// swap():"" -> ""
// x() -> 1, 2
// x():"" -> "1, 2"
// y() -> 1, 2
// y():"" -> "1, 2"
