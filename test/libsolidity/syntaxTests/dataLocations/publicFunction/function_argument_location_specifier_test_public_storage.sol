contract test {
    function f(bytes storage) public;
}
// ----
// TypeError: (31-36): Storage location must be "memory" for parameter in public function, but "storage" was given.
