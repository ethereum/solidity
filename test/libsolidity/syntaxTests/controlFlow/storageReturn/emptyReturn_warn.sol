contract C {
    struct S { bool f; }
    S s;
    function f() internal pure returns (S storage) { return; }
    function g() internal view returns (S storage c, S storage) { c = s; return; }
    function h() internal view returns (S storage, S storage d) { d = s; return; }
    function i() internal pure returns (S storage, S storage) { return; }
    function j() internal view returns (S storage, S storage) { return (s,s); }
}
// ----
// Warning: (87-88): This variable is of storage pointer type and might be returned without assignment. This can cause storage corruption. Assign the variable (potentially from itself) to remove this warning.
// Warning: (163-164): This variable is of storage pointer type and might be returned without assignment. This can cause storage corruption. Assign the variable (potentially from itself) to remove this warning.
// Warning: (233-234): This variable is of storage pointer type and might be returned without assignment. This can cause storage corruption. Assign the variable (potentially from itself) to remove this warning.
// Warning: (316-317): This variable is of storage pointer type and might be returned without assignment. This can cause storage corruption. Assign the variable (potentially from itself) to remove this warning.
// Warning: (327-328): This variable is of storage pointer type and might be returned without assignment. This can cause storage corruption. Assign the variable (potentially from itself) to remove this warning.
