contract C {
    function f() external {}
    function g() external {}
    function h() pure external {}
    function i() view external {}

    function comparison_operators_for_external_functions() public returns (bool) {
        assert(
            this.f != this.g &&
            this.f != this.h &&
            this.f != this.i &&

            this.g != this.h &&
            this.g != this.i &&

            this.h != this.i &&

            this.f == this.f &&
            this.g == this.g &&
            this.h == this.h &&
            this.i == this.i
        );
        return true;
    }

    function comparison_operators_for_local_external_function_pointers() public returns (bool) {
        function () external f_local = this.f;
        function () external g_local = this.g;
        function () external pure h_local = this.h;
        function () external view i_local = this.i;

        assert(
            f_local == this.f &&
            g_local == this.g &&
            h_local == this.h &&
            i_local == this.i &&

            f_local != this.g &&
            f_local != this.h &&
            f_local != this.i &&

            g_local != this.f &&
            g_local != this.h &&
            g_local != this.i &&

            h_local != this.f &&
            h_local != this.g &&
            h_local != this.i &&

            i_local != this.f &&
            i_local != this.g &&
            i_local != this.h
        );

        assert(
            f_local == f_local &&
            f_local != g_local &&
            f_local != h_local &&
            f_local != i_local
        );

        assert(
            g_local == g_local &&
            g_local != h_local &&
            g_local != i_local
        );

        assert(
            h_local == h_local &&
            i_local == i_local &&
            h_local != i_local
        );

        return true;
    }
}
// ----
// comparison_operators_for_external_functions() -> true
// comparison_operators_for_local_external_function_pointers() -> true
