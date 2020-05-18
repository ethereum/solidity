object "A" {
  code {
    pop(dataoffset("A"))
    pop(dataoffset("B"))
    pop(dataoffset("B.C"))
    pop(dataoffset("B.C.D"))
    pop(dataoffset("B.D"))    // invalid
    pop(dataoffset("C.D"))    // invalid
  }

  object "B" {
    code {
      pop(dataoffset("C"))
      pop(dataoffset("C.D"))
    }
    object "C" {
      code {
        pop(dataoffset("D"))
      }
      object "D" {
        code {
          invalid()
        }
      }
    }
  }
}
