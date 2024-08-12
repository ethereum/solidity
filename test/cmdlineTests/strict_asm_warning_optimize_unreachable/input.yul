{
    revert(0, 0)

    // The code is valid and will proceed through the whole pipeline, possibly being reparsed multiple times.
    // Note that the code is unreachable, so the warning would not appear in the optimized version.
    // This should produce exactly one warning.
    selfdestruct(0)
}
