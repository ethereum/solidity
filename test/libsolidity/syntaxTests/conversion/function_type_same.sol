contract C {
	int dummy;
    function h_nonpayable() external { dummy = 1; }
    function h_payable() payable external {}
    function h_view() view external { dummy; }
    function h_pure() pure external {}
    function f() view external {
        function () external g_nonpayable = this.h_nonpayable; g_nonpayable;
        function () payable external g_payable = this.h_payable; g_payable;
        function () view external g_view = this.h_view; g_view;
        function () pure external g_pure = this.h_pure; g_pure;
    }
}
// ----
