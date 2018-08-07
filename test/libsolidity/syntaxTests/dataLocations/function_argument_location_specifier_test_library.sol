library test {
    function f(bytes calldata) public;
}
// ----
// TypeError: (30-35): Location has to be memory or storage for public function in library.
