contract C {
    function external_test_function() external {}
    function comparison_operator_for_external_function_with_extra_slots() external returns (bool) {
        return (
            (this.external_test_function{gas: 4} == this.external_test_function) &&
            (this.external_test_function{gas: 4} == this.external_test_function{gas: 4})
        );
    }
}
// ----
// TypeError 2271: (193-259): Binary operator == not compatible with types function () external and function () external.
// TypeError 2271: (277-351): Binary operator == not compatible with types function () external and function () external.
