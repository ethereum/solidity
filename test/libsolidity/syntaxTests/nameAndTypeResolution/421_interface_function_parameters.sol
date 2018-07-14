interface I {
    function f(uint a) public returns (bool);
}
// ----
// TypeError: (18-59): Functions in interfaces must be declared external.
