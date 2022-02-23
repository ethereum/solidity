function fun() {
    fun{gas: 1}();
    fun{value: 1}();
}
// ----
// TypeError 2193: (21-32): Function call options can only be set on external function calls or contract creations.
