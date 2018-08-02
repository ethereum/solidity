library test {
    function f(bytes calldata) public;
}
// ----
// TypeError: (30-35): Storage location must be one of "storage", "memory" for parameter in public function, but "calldata" was given.
