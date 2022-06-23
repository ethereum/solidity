contract Contract {
    struct S {
        mapping(uint => address) map;
    }

    S immutable s;
}
// ----
// TypeError 6377: (84-97): Immutable variables cannot have a non-value type.
