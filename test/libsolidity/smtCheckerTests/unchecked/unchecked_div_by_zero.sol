contract C {
    function div(uint256 a, uint256 b) public pure returns (uint256) {
        // Does not disable div by zero check
        unchecked {
            return a / b;
        }
    }

    function mod(uint256 a, uint256 b) public pure returns (uint256) {
        // Does not disable div by zero check
        unchecked {
            return a % b;
        }
    }
}
// ====
// SMTEngine: all
// ----
// Warning 4281: (169-174): CHC: Division by zero happens here.\nCounterexample:\n\na = 0\nb = 0\n = 0\n\nTransaction trace:\nC.constructor()\nC.div(0, 0)
// Warning 4281: (349-354): CHC: Division by zero happens here.\nCounterexample:\n\na = 0\nb = 0\n = 0\n\nTransaction trace:\nC.constructor()\nC.mod(0, 0)
