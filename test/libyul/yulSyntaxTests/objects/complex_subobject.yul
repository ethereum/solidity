object "A" {
  code {}
  data "1" hex"001122"

  object "B" {
    code {}
    data "2" ""

    object "B_C" {
      code {}

      object "B_C_1" {
        code {}
      }
      object "B_C_2" {
        code {}
      }
    }
  }

  object "C" {
    code {}
  }

  object "D" {
    code {}

    object "D_1" {
      code {}
    }
    object "D_2" {
      code {}
    }
  }
}
