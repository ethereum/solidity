contract C {
    enum EnumType {A, B, C}

    struct StructType {
        uint x;
    }

    event E1(StructType StructType);
    event E2(EnumType EnumType);
    event E3(EnumType StructType, StructType EnumType);
    event E4(StructType indexed StructType) anonymous;
}
// ----
