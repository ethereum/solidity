contract C {
    enum Direction { Left, Right }
}

contract D is C {
    function f() public pure returns (Direction) {
      return Direction.Left;
    }
}
