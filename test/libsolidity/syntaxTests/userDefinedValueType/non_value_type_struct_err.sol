struct S {uint x;}

contract C {
    type MyType is S;
}
// ----
// TypeError 8657: (52-53='S'): The underlying type for a user defined value type has to be an elementary value type.
