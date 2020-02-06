library L {}
contract C { mapping(L => bool) i; }
// ----
// TypeError: (34-35): Library types cannot be used as mapping keys.
