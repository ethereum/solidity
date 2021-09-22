/// @use-src 0:"input.sol"
object "C" {
    code {
        let x, y := 1
    }
}
// ----
// DeclarationError 3812: (59-72): Variable count mismatch for declaration of "x, y": 2 variables and 1 values.
