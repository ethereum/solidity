object "A" {
  code {
    pop(dataoffset("B.C"))
    pop(datasize("B.C"))
    pop(dataoffset("D.E.F.G"))
    pop(datasize("D.E.F.G"))
  }

  object "B" {
    code {}
    data "C" hex"00"
  }

  object "D" {
    code {}
    object "E" {
      code {}
      object "F" {
        code{}
        data "G" ""
      }
    }
  }
}
// ----
