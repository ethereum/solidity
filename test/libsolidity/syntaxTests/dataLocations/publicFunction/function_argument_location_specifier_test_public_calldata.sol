contract test {
    function f(bytes calldata) public;
}
// ----
// TypeError: (31-36): Storage location must be "memory" for parameter in public function, but "calldata" was given.
