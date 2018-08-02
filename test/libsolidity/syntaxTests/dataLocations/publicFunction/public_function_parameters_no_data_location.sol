contract C {
    function h(uint[]) public pure {}
}
// ----
// TypeError: (28-34): Storage location must be one of "memory" for parameter in public function, but none was given.
