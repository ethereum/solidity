contract C {
    function external_test_function1(uint num) external {}
    function external_test_function2(bool val) external {}

    function comparison_operator_between_internal_and_external_function_pointers() external returns (bool) {
        function () external external_function_pointer_local1 = this.external_test_function1;
        function () external external_function_pointer_local2 = this.external_test_function2;

        assert(
            this.external_test_function1 == external_function_pointer_local1 &&
            this.external_test_function2 == external_function_pointer_local2
        );
        assert(
            external_function_pointer_local2 != external_function_pointer_local1 &&
            this.external_test_function2 != this.external_test_function1
        );

        return true;
    }
}
// ----
// TypeError 9574: (249-333): Type function (uint256) external is not implicitly convertible to expected type function () external.
// TypeError 9574: (343-427): Type function (bool) external is not implicitly convertible to expected type function () external.
// TypeError 2271: (458-522): Binary operator == not compatible with types function (uint256) external and function () external.
// TypeError 2271: (538-602): Binary operator == not compatible with types function (bool) external and function () external.
// TypeError 2271: (726-786): Binary operator != not compatible with types function (bool) external and function (uint256) external.
