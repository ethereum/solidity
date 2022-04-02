function f() pure {
    assembly ("a", "b", "c", "c") {}
}
// ----
// Warning 4430: (24-56='assembly ("a", "b", "c", "c") {}'): Unknown inline assembly flag: "a"
// Warning 4430: (24-56='assembly ("a", "b", "c", "c") {}'): Unknown inline assembly flag: "b"
// Warning 4430: (24-56='assembly ("a", "b", "c", "c") {}'): Unknown inline assembly flag: "c"
// Warning 4430: (24-56='assembly ("a", "b", "c", "c") {}'): Unknown inline assembly flag: "c"
