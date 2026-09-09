contract StripReasonPanic {
    enum MyEnum { A, B }
    string constant ARRAY_OUT_OF_BOUNDS_REASON = (new bytes(0))[0] == bytes1(0) ? "first" : "second";
    string constant OVERFLOW_REASON = type(uint8).max + 1 == 0 ? "first" : "second";
    string constant DIVISION_BY_ZERO_REASON = 1 / type(uint8).min == 0 ? "first" : "second";
    string constant ENUM_CONVERSION_REASON = MyEnum(type(uint8).max) == MyEnum.A ? "first" : "second";

    function requireReasonArrayOutOfBoundsIsEvaluated() external returns (uint256) {
        require(true, (new bytes(0))[0] == bytes1(0) ? "first" : "second");
        return 1;
    }

    function revertReasonArrayOutOfBoundsIsEvaluated() external returns (uint256) {
        revert((new bytes(0))[0] == bytes1(0) ? "first" : "second");
    }

    function requireReasonOverflowIsEvaluated() external returns (uint256) {
        require(true, type(uint8).max + 1 == 0 ? "first" : "second");
        return 1;
    }

    function revertReasonOverflowIsEvaluated() external returns (uint256) {
        revert(type(uint8).max + 1 == 0 ? "first" : "second");
    }

    function requireReasonDivisionByZeroIsEvaluated() external returns (uint256) {
        require(true, 1 / type(uint8).min == 0 ? "first" : "second");
        return 1;
    }

    function revertReasonDivisionByZeroIsEvaluated() external returns (uint256) {
        revert(1 / type(uint8).min == 0 ? "first" : "second");
    }

    function requireReasonEnumConversionIsEvaluated() external returns (uint256) {
        require(true, MyEnum(type(uint8).max) == MyEnum.A ? "first" : "second");
        return 1;
    }

    function revertReasonEnumConversionIsEvaluated() external returns (uint256) {
        revert(MyEnum(type(uint8).max) == MyEnum.A ? "first" : "second");
    }

    function requireReasonConstantIsEvaluated() external returns (uint256) {
        require(true, ARRAY_OUT_OF_BOUNDS_REASON);
        return 1;
    }

    function revertReasonConstantIsEvaluated() external returns (uint256) {
        revert(ARRAY_OUT_OF_BOUNDS_REASON);
    }

    function requireReasonOverflowConstantIsEvaluated() external returns (uint256) {
        require(true, OVERFLOW_REASON);
        return 1;
    }

    function revertReasonOverflowConstantIsEvaluated() external returns (uint256) {
        revert(OVERFLOW_REASON);
    }

    function requireReasonDivisionByZeroConstantIsEvaluated() external returns (uint256) {
        require(true, DIVISION_BY_ZERO_REASON);
        return 1;
    }

    function revertReasonDivisionByZeroConstantIsEvaluated() external returns (uint256) {
        revert(DIVISION_BY_ZERO_REASON);
    }

    function requireReasonEnumConversionConstantIsEvaluated() external returns (uint256) {
        require(true, ENUM_CONVERSION_REASON);
        return 1;
    }

    function revertReasonEnumConversionConstantIsEvaluated() external returns (uint256) {
        revert(ENUM_CONVERSION_REASON);
    }

    function requireReasonNestedConditionalTrueBranchIsEvaluated() external returns (uint256) {
        require(
            true,
            true
                ? (type(uint8).max + 1 == 0 ? "first" : "second")
                : (1 / type(uint8).min == 0 ? "third" : "fourth")
        );
        return 1;
    }

    function revertReasonNestedConditionalTrueBranchIsEvaluated() external returns (uint256) {
        revert(
            true
                ? (type(uint8).max + 1 == 0 ? "first" : "second")
                : (1 / type(uint8).min == 0 ? "third" : "fourth")
        );
    }

    function requireReasonNestedConditionalFalseBranchIsEvaluated() external returns (uint256) {
        require(
            true,
            false
                ? (type(uint8).max + 1 == 0 ? "first" : "second")
                : (1 / type(uint8).min == 0 ? "third" : "fourth")
        );
        return 1;
    }

    function revertReasonNestedConditionalFalseBranchIsEvaluated() external returns (uint256) {
        revert(
            false
                ? (type(uint8).max + 1 == 0 ? "first" : "second")
                : (1 / type(uint8).min == 0 ? "third" : "fourth")
        );
    }
}
// ====
// EVMVersion: >=byzantium
// revertStrings: strip
// ----
// requireReasonArrayOutOfBoundsIsEvaluated() -> FAILURE, hex"4e487b71", 0x32
// revertReasonArrayOutOfBoundsIsEvaluated() -> FAILURE, hex"4e487b71", 0x32
// requireReasonOverflowIsEvaluated() -> FAILURE, hex"4e487b71", 0x11
// revertReasonOverflowIsEvaluated() -> FAILURE, hex"4e487b71", 0x11
// requireReasonDivisionByZeroIsEvaluated() -> FAILURE, hex"4e487b71", 0x12
// revertReasonDivisionByZeroIsEvaluated() -> FAILURE, hex"4e487b71", 0x12
// requireReasonEnumConversionIsEvaluated() -> FAILURE, hex"4e487b71", 0x21
// revertReasonEnumConversionIsEvaluated() -> FAILURE, hex"4e487b71", 0x21
// requireReasonConstantIsEvaluated() -> FAILURE, hex"4e487b71", 0x32
// revertReasonConstantIsEvaluated() -> FAILURE, hex"4e487b71", 0x32
// requireReasonOverflowConstantIsEvaluated() -> FAILURE, hex"4e487b71", 0x11
// revertReasonOverflowConstantIsEvaluated() -> FAILURE, hex"4e487b71", 0x11
// requireReasonDivisionByZeroConstantIsEvaluated() -> FAILURE, hex"4e487b71", 0x12
// revertReasonDivisionByZeroConstantIsEvaluated() -> FAILURE, hex"4e487b71", 0x12
// requireReasonEnumConversionConstantIsEvaluated() -> FAILURE, hex"4e487b71", 0x21
// revertReasonEnumConversionConstantIsEvaluated() -> FAILURE, hex"4e487b71", 0x21
// requireReasonNestedConditionalTrueBranchIsEvaluated() -> FAILURE, hex"4e487b71", 0x11
// revertReasonNestedConditionalTrueBranchIsEvaluated() -> FAILURE, hex"4e487b71", 0x11
// requireReasonNestedConditionalFalseBranchIsEvaluated() -> FAILURE, hex"4e487b71", 0x12
// revertReasonNestedConditionalFalseBranchIsEvaluated() -> FAILURE, hex"4e487b71", 0x12