object "A" {
  code {
    pop(dataoffset("C"))
  }
  data "B" ""
}
// ----
// TypeError 3517: (41-44='"C"'): Unknown data object "C".
