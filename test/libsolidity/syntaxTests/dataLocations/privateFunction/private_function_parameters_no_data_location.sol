contract C {
    function f(uint[]) private pure {}
}
// ----
// TypeError 6651: (28-34): Data location must be "storage", "memory" or "calldata" for parameter in private function, but none was given.
