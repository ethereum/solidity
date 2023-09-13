contract C {
    function f() public pure {
        /// TestCase1: condition.operator
        /// TestCase2: initializationExpression.initialValue.value
        /// TestCase3: loopExpression.expression.subExpression.name
        for(uint i = 1; i < 42; i++) {
        }
    }
}
// ----
// TestCase1: <
// TestCase2: 1
// TestCase3: i
