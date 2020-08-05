object "A" {
  code {
    pop(dataoffset("B"))
    pop(datasize("B"))
  }

  object "B" {
    code {}
  }
}
// ----
