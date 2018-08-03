library L {
    function h(uint[]) public pure {}
}
// ----
// TypeError: (27-33): Storage location must be "storage" or "memory" for parameter in public function, but none was given.
