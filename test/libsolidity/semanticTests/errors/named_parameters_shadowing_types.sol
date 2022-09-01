pragma abicoder v2;

contract C {
    enum EnumType {A, B, C}

    struct StructType {
        uint x;
    }

    error E1(StructType StructType);
    error E2(EnumType StructType, StructType EnumType);

    function f() public {
        revert E1({StructType: StructType(42)});
    }

    function g() public {
        revert E2({EnumType: StructType(42), StructType: EnumType.B});
    }
}
// ====
// compileToEwasm: also
// ----
// f() -> FAILURE, hex"33a54193", hex"000000000000000000000000000000000000000000000000000000000000002a"
// g() -> FAILURE, hex"374b9387", hex"0000000000000000000000000000000000000000000000000000000000000001", hex"000000000000000000000000000000000000000000000000000000000000002a"
