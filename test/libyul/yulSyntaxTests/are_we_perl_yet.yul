{
    function _...($..) {}
    let a...
    _...(a...)
}
// ----
// SyntaxError 3384: (6-27): "_..." is not a valid identifier (ends with a dot).
// SyntaxError 7771: (6-27): "_..." is not a valid identifier (contains consecutive dots).
// SyntaxError 3384: (20-23): "$.." is not a valid identifier (ends with a dot).
// SyntaxError 7771: (20-23): "$.." is not a valid identifier (contains consecutive dots).
// SyntaxError 3384: (36-40): "a..." is not a valid identifier (ends with a dot).
// SyntaxError 7771: (36-40): "a..." is not a valid identifier (contains consecutive dots).
