/// @use-src 0:"input.sol"
object "C" {
    code {
        /// @src 0:0:0
        calldataload(0)
    }
}
// ----
// TypeError 3083: (82-97): Top-level expressions are not supposed to return values (this expression returns 1 value). Use ``pop()`` or assign them.
