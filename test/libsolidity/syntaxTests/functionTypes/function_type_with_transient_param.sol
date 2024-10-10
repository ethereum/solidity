contract C {
    function (uint transient) external y;
    function (uint[] transient) external z;
}
// ----
// Warning 6162: (27-41): Naming function type parameters is deprecated.
// Warning 6162: (69-85): Naming function type parameters is deprecated.
// TypeError 6651: (69-85): Data location must be "memory" or "calldata" for parameter in function, but none was given.
