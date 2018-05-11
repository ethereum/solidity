contract test {
    function f(uint[] memory constant a) public { }
}
// ----
// TypeError: (31-55): Illegal use of "constant" specifier.
// TypeError: (31-55): Constants of non-value type not yet implemented.
// TypeError: (31-55): Uninitialized "constant" variable.
