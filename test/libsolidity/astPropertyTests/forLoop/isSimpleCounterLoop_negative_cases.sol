type UINT is uint;

function add(UINT x, UINT y) pure returns (UINT) {
    return UINT.wrap(UINT.unwrap(x) + UINT.unwrap(y));
}

function lt(UINT x, UINT y) pure returns (bool) {
    return UINT.unwrap(x) < UINT.unwrap(y);
}

using {lt as <, add as +} for UINT global;

function g() pure returns (bool) {
    return false;
}

function h() pure returns (uint) {
    return 13;
}

contract C {
    uint z = 0;

    function modifyStateVarZ() public returns (uint) {
        z = type(uint).max;
        return z;
    }

    struct S {
        uint x;
    }

    function f() public {
        /// AdditionLoopExpression: isSimpleCounterLoop
        for(uint i = 0; i < 42; i = i + 1) {
        }
        /// ShortHandAdditionLoopExpression: isSimpleCounterLoop
        for(uint i = 0; i < 42; i += 1) {
        }
        /// SimplePreDecrement: isSimpleCounterLoop
        for(uint i = 42; i > 0; --i) {
        }
        /// SimplePosDecrement: isSimpleCounterLoop
        for(uint i = 42; i > 0; i--) {
        }
        /// MultiplicationLoopExpression: isSimpleCounterLoop
        for(uint i = 1; i < 42; i = i * 2) {
        }
        /// CounterIncrementLoopBody: isSimpleCounterLoop
        for(uint i = 0; i < 42; ++i) {
            i++;
        }
        /// CounterAssignmentLoopBody: isSimpleCounterLoop
        for(uint i = 0; i < 42; ++i) {
            i = 43;
        }
        /// CounterAssignmentInlineAssemblyLoopBody: isSimpleCounterLoop
        for(uint i = 0; i < 42; ++i) {
            assembly {
                i := add(i, 1)
            }
        }
        uint j = type(uint).max;
        /// ExternalCounterLoopExpression: isSimpleCounterLoop
        for (uint i = 0; i < 10; ++j) {
        }
        uint x;
        /// CounterIncrementRHSAssignment: isSimpleCounterLoop
        for(uint i = 0; i < 10; ++i) {
            x = i++;
        }
        /// NoEffectLoopExpression: isSimpleCounterLoop
        for(uint i = 0; i < 42; i) {
        }
        /// EmptyLoopExpression: isSimpleCounterLoop
        for(uint i = 0; i < 10; ) {
        }
        j = 0;
        /// EmptyConditionExpression: isSimpleCounterLoop
        for(uint i = 0; ; ++i) {
        }
        uint y = type(uint8).max + 1;
        /// DifferentCommonTypeCondition: isSimpleCounterLoop
        for(uint8 i = 0; i < y; ++i) {
        }
        /// LessThanOrEqualCondition: isSimpleCounterLoop
        for(uint i = 0; i <= 10; ++i) {
        }
        /// ComplexExpressionCondition: isSimpleCounterLoop
        for(uint i = 0; (i < 10 || g()); ++i) {
        }
        /// FreeFunctionConditionLHS: isSimpleCounterLoop
        for(uint i = 0; h() < 100; ++i) {
        }
        /// FreeFunctionConditionDifferentCommonTypeLHS: isSimpleCounterLoop
        for(uint8 i = 0; i < h(); ++i) {
        }
        /// NonIntegerTypeCondition: isSimpleCounterLoop
        for(uint i = 0; address(this) < msg.sender; ++i) {
        }
        UINT ZERO = UINT.wrap(0);
        UINT ONE = UINT.wrap(1);
        UINT TEN = UINT.wrap(10);
        /// UDVTOperators: isSimpleCounterLoop
        for(UINT i = ZERO; i < TEN; i = i + ONE) {
        }
        /// CounterAssignmentConditionRHS: isSimpleCounterLoop
        for(uint i = 0; i < (i = i + 1); ++i) {
        }
        /// LiteralDifferentCommonTypeConditionRHS: isSimpleCounterLoop
        for(uint8 i = 0; i < 257 ; ++i) {
        }
        /// StateVarCounterModifiedFunctionConditionRHS: isSimpleCounterLoop
        for (z = 1; z < modifyStateVarZ(); ++z) {
        }
        /// StateVarCounterModifiedFunctionLoopBody: isSimpleCounterLoop
        for (z = 1; z < 2048; ++z) {
            modifyStateVarZ();
        }
        /// NonIntegerCounter: isSimpleCounterLoop
        for (address i = address(0x123); i < address(this); i = address(0x123 + 1)) {
        }
        /// AssemblyAfterAssignmentToCounter: isSimpleCounterLoop
        for (uint i = 0; i < 42; ++i) {
            i = type(uint).max;
            assembly {}
        }
        uint k = type(uint).max;
        /// ExpressionIncrement: isSimpleCounterLoop
        for (uint i = 0; i < 10; ++(k)) {
        }
        S memory s = S(type(uint).max);
        /// StructMemberAccessIncrement: isSimpleCounterLoop
        for (uint i = 0; i < 10; ++s.x) {
        }
    }
}
// ----
// AdditionLoopExpression: false
// ShortHandAdditionLoopExpression: false
// SimplePreDecrement: false
// SimplePosDecrement: false
// MultiplicationLoopExpression: false
// CounterIncrementLoopBody: false
// CounterAssignmentLoopBody: false
// CounterAssignmentInlineAssemblyLoopBody: false
// ExternalCounterLoopExpression: false
// CounterIncrementRHSAssignment: false
// NoEffectLoopExpression: false
// DifferentCommonTypeCondition: false
// EmptyLoopExpression: false
// EmptyConditionExpression: false
// LessThanOrEqualCondition: false
// ComplexExpressionCondition: false
// FreeFunctionConditionLHS: false
// FreeFunctionConditionDifferentCommonTypeLHS: false
// NonIntegerTypeCondition: false
// UDVTOperators: false
// CounterAssignmentConditionRHS: false
// LiteralDifferentCommonTypeConditionRHS: false
// StateVarCounterModifiedFunctionConditionRHS: false
// StateVarCounterModifiedFunctionLoopBody: false
// NonIntegerCounter: false
// AssemblyAfterAssignmentToCounter: false
// ExpressionIncrement: false
// StructMemberAccessIncrement: false
