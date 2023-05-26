/// @use-src 0:"input.sol"
object "C_6_deployed" {
    code {
        /// @src 0:60:101  "contract C {..."
        mstore(64, 128)

        // f()
        fun_f_5()

        /// @src 0:77:99  "function f() public {}"
        function fun_f_5() {
        }
        /// @src 0:60:101  "contract C {..."
    }
}
