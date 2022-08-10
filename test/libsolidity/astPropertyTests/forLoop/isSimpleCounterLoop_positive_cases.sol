function h() pure returns (uint) {
    return 13;
}

contract C {
    uint[] dynArray;

    function f() public {
        /// SimplePreIncrement: isSimpleCounterLoop
        for(uint i = 0; i < 42; ++i) {
        }
        /// SimplePosIncrement: isSimpleCounterLoop
        for(int i = 0; i < 42; i++) {
        }
        uint x;
        /// CounterReadLoopBody: isSimpleCounterLoop
        for(uint i = 0; i < 42; i++) {
            x = i;
        }
        /// LocalVarConditionRHS: isSimpleCounterLoop
        for(uint i = 0; i < x; i++) {
        }
        x = 0;
        /// EmptyInitExpression: isSimpleCounterLoop
        for(; x < 10; ++x) {
        }
        uint[8] memory array;
        /// StaticArrayLengthConditionRHS: isSimpleCounterLoop
        for(uint i = 0; i < array.length; i++) {
        }
        dynArray.push();
        /// DynamicArrayLengthConditionRHS: isSimpleCounterLoop
        for(uint i = 0; i < dynArray.length; i++) {
            dynArray.push(i);
        }
        /// CounterReadInlineAssembly: isSimpleCounterLoop
        for(uint i = 0; i < 42; ++i) {
            assembly {
                x := i
            }
        }
        /// BinaryOperationConditionRHS: isSimpleCounterLoop
        for(uint i = 0; i < i + 1; i++) {
        }
        /// FreeFunctionConditionRHS: isSimpleCounterLoop
        for(uint i = 0; i < h(); ++i) {
        }
    }
    function functionParamLoopCounter(uint i) public pure {
        /// FunctionParameterLoopCounter: isSimpleCounterLoop
        for (i = 0; i < 42; ++i){
        }
    }
    function functionReturnLoopCounter() public pure returns (uint i) {
        /// FunctionReturnLoopCounter: isSimpleCounterLoop
        for (i = 0; i < 42; ++i){
        }
    }
}
// ----
// SimplePreIncrement: true
// SimplePosIncrement: true
// CounterReadLoopBody: true
// LocalVarConditionRHS: true
// EmptyInitExpression: true
// StaticArrayLengthConditionRHS: true
// DynamicArrayLengthConditionRHS: true
// CounterReadInlineAssembly: true
// BinaryOperationConditionRHS: true
// FreeFunctionConditionRHS: true
// FunctionParameterLoopCounter: true
// FunctionReturnLoopCounter: true
