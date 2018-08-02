contract test {
    function f(bytes calldata) public;
}
// ----
// TypeError: (31-36): Storage location must be one of "memory" for parameter in public function, but "calldata" was given.
