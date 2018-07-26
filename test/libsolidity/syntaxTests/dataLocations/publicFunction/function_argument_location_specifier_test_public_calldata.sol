contract test {
    function f(bytes calldata) public;
}
// ----
// TypeError: (31-36): Location has to be memory for public function. Remove the data location keyword to fix this error.
