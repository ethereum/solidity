library test {
    function f(bytes calldata) public;
}
// ----
// TypeError: (30-35): Storage location must be "storage" or "memory" for parameter in public function, but "calldata" was given.
