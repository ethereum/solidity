interface I {
    enum Direction { Left, Right }
}

library L {
    function f() public pure returns (I.Direction) {
      return I.Direction.Left;
    }
    function g() internal pure returns (I.Direction) {
      return I.Direction.Left;
    }
}
