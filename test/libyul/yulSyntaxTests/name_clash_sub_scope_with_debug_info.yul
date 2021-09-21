/// @use-src 0:"input.sol"
object "C" {
    code {
        /// @src 0:0:0
        function g() {
            function g() {}
        }
    }
}
// ----
// DeclarationError 6052: (0-0): Function name g already taken in this scope.
