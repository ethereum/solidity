type MyInt1 is MyInt2;
type MyInt2 is MyInt1;
// ----
// TypeError 8657: (15-21): The underlying type for a user defined value type has to be an elementary value type.
