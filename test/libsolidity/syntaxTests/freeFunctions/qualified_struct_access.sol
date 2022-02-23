function f() returns (uint) { C.S storage t; t.x; }

contract C {
    struct S { uint x; }
}
// ----
// Warning 6321: (22-26): Unnamed return variable can remain unassigned. Add an explicit return with value to all non-reverting code paths or name the variable.
// TypeError 3464: (45-46): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
