contract C {
    enum EnumType {A, B, C}

    struct StructType {
        uint x;
    }

    error E1(StructType StructType);
    error E2(EnumType EnumType);
    error E3(EnumType StructType, StructType EnumType);
}
// ----
