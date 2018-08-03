contract C {
    struct S { bool f; }
    S s;
    function f() internal pure returns (S storage) { return; }
    function g() internal view returns (S storage c, S storage) { c = s; return; }
    function h() internal view returns (S storage, S storage d) { d = s; return; }
    function i() internal pure returns (S storage, S storage) { return; }
}
// ----
// TypeError: (87-88): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
// TypeError: (163-164): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
// TypeError: (233-234): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
// TypeError: (316-317): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
// TypeError: (327-328): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
