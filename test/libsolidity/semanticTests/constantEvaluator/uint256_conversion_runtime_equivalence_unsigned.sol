uint constant UNSIGNED_8 = uint(8);

contract C {
    uint constant LITERAL = uint(42);
    uint constant CONST = uint(UNSIGNED_8);
    uint constant CHAINED = uint(uint(uint(64)));
    uint constant EXPRESSION = uint((CONST + LITERAL) * 2);

    int[LITERAL] a;
    int[CONST] b;
    int[CHAINED] c;
    int[EXPRESSION] d;

    function testLiteralEquivalence() public view returns (bool) {
        uint runtime = uint(42);
        return a.length == runtime;
    }
    function testConstEquivalence() public view returns (bool) {
        uint runtime = uint(UNSIGNED_8);
        return b.length == runtime;
    }
    function testChainedEquivalence() public view returns (bool) {
        uint runtime = uint(uint(uint(64)));
        return c.length == runtime;
    }
    function testExpressionEquivalence() public view returns (bool) {
        uint runtime = uint((CONST + LITERAL) * 2);
        return d.length == runtime;
    }
}
// ----
// testLiteralEquivalence() -> true
// testConstEquivalence() -> true
// testChainedEquivalence() -> true
// testExpressionEquivalence() -> true
