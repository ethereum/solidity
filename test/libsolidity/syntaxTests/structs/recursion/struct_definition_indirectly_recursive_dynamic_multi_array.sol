contract Test {
    struct S1 {
        S2[1][] x;
    }
    struct S2 {
        S1 x;
    }
    struct T1 {
        T2[][1] x;
    }
    struct T2 {
        T1 x;
    }
    struct R1 {
        R2[][] x;
    }
    struct R2 {
        R1 x;
    }
}
// ----
