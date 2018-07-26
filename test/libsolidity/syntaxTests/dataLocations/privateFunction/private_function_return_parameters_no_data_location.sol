contract C {
    function f() private pure returns(uint[]) {}
}
// ----
// TypeError: (51-57): Storage location must be one of: "storage", "memory" for parameter in private function, but none was given.
