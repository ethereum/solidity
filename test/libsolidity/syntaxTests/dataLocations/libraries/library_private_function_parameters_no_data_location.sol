library L {
    function h(uint[]) public pure {}
}
// ----
// TypeError: (27-33): Storage location must be one of "storage", "memory" for parameter in public function, but none was given.
