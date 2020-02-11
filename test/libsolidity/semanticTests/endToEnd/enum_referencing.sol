interface I {
    enum Direction {
        A,
        B,
        Left,
        Right
    }
}
library L {
    enum Direction {
        Left,
        Right
    }

    function f() public pure returns(Direction) {
        return Direction.Right;
    }

    function g() public pure returns(I.Direction) {
        return I.Direction.Right;
    }
}
// ----
// f() -> 1
// g() -> 3
