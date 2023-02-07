enum E {A, B, C}

type MyType is E;
// ----
// TypeError 8657: (33-34): The underlying type for a user defined value type has to be an elementary value type.
