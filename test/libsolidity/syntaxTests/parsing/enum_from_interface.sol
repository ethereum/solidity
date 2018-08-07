interface I {
    enum Direction { Left, Right }
}

contract D {
    function f() public pure returns (I.Direction) {
      return I.Direction.Left;
    }
}
