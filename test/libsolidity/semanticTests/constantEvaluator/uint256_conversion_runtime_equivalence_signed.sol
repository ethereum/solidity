int constant NEGATIVE = -(2**128);
int constant POSITIVE = 2**64;

contract C {
    int[uint(NEGATIVE)] fromNegative;
    int[uint(POSITIVE)] fromPositive;
    int[2**256 - 1 - (uint(NEGATIVE) + uint(POSITIVE))] fromExpression;

    function testNegativeIntEquivalence() public view returns (bool) {
        uint runtime = uint(NEGATIVE);

        return fromNegative.length == runtime;
    }
    function testPositiveIntEquivalence() public view returns (bool) {
        uint runtime = uint(POSITIVE);

        return fromPositive.length == runtime;
    }
    function testExpressionEquivalence() public view returns (bool) {
        uint runtime = 2**256 - 1 - (uint(NEGATIVE) + uint(POSITIVE));

        return fromExpression.length == runtime;
    }
}
// ----
// testNegativeIntEquivalence() -> true
// testPositiveIntEquivalence() -> true
// testExpressionEquivalence() -> true
