contract test {
    function f(bytes storage) public;
}
// ----
// TypeError: (31-36): Storage location must be one of "memory" for parameter in public function, but "storage" was given.
