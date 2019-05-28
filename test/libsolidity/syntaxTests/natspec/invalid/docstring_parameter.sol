contract C {
    /**
     * @param 5value a value parameter
     * @param _ a value parameter
     */
    function f(uint256 _5value, uint256 _value, uint256 value) internal {
    }
}
// ----
// DocstringParsingError: Documented parameter "" not found in the parameter list of the function.
// DocstringParsingError: Documented parameter "_" not found in the parameter list of the function.
