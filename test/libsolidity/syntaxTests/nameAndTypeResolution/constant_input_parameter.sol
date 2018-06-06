contract test {
    function f(uint[] constant a) public { }
}
// ----
// TypeError: (31-48): Illegal use of "constant" specifier.
// TypeError: (31-48): Constants of non-value type not yet implemented.
// TypeError: (31-48): Uninitialized "constant" variable.
