function f(MyIntB x) pure {}
type MyIntB is MyIntB;
// ----
// TypeError 8657: (44-50): The underlying type for a user defined value type has to be an elementary value type.
