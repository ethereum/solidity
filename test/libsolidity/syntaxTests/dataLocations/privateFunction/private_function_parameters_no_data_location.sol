contract C {
    function f(uint[]) private pure {}
}
// ----
// TypeError: (28-34): Storage location must be "storage" or "memory" for parameter in private function, but none was given.
