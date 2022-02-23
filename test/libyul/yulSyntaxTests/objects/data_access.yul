object "A" {
  code {
    pop(dataoffset("B"))
    pop(datasize("B"))
  }

  data "B" hex"00"
}
// ----
