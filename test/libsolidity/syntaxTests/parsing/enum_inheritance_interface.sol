interface I {
    enum Direction { Left, Right }
}

contract D is I {
    function f() public pure returns (Direction) {
      return Direction.Left;
    }
}
