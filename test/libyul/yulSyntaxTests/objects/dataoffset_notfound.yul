object "A" {
  code {
    pop(dataoffset("C"))
  }
  data "B" ""
}
// ----
// TypeError 3517: (41-44): Unknown data object "C".
