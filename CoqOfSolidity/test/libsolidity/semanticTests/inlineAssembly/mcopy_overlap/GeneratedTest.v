(* Generated test file *)
Require Import CoqOfSolidity.CoqOfSolidity.
Require Import simulations.CoqOfSolidity.

Require test.libsolidity.semanticTests.inlineAssembly.mcopy_overlap.C.

Definition constructor_code : Code.t :=
  test.libsolidity.semanticTests.inlineAssembly.mcopy_overlap.C.C.code.

Definition deployed_code : Code.t :=
  test.libsolidity.semanticTests.inlineAssembly.mcopy_overlap.C.C.deployed.code.

Definition codes : list Code.t :=
  test.libsolidity.semanticTests.inlineAssembly.mcopy_overlap.C.codes.

Module Constructor.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := [];
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
    Environment.code_name := constructor_code.(Code.hex_name);
  |}.

  Definition initial_state : State.t :=
    let address := environment.(Environment.address) in
    let account := {|
      Account.balance := environment.(Environment.callvalue);
      Account.nonce := 1;
      Account.code := constructor_code.(Code.hex_name);
      Account.codedata := Memory.hex_string_as_bytes "";
      Account.storage := Memory.empty;
      Account.immutables := [];
    |} in
    State.init <| State.accounts := [(address, account)] |>.

  Definition result_state :=
    eval_with_revert 5000 codes environment constructor_code.(Code.body) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Goal Test.is_return result state = inl (Memory.u256_as_bytes deployed_code.(Code.hex_name)).
  Proof.
    vm_compute.
    reflexivity.
  Qed.

Definition final_state : State.t :=
  snd (
    eval 5000 codes environment (update_current_code_for_deploy deployed_code.(Code.hex_name)) state
  ).
End Constructor.

(* // mcopy_to_right_overlap() -> 0x20, 0x60, 0x2222222222222222333333333333333344444444444444445555555555555555, 0x4444444444444444555555555555555566666666666666667777777777777777, 0x88888888888888889999999999999999ccccccccccccccccdddddddddddddddd *)
Module Step1.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "afdce848";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
    Environment.code_name := deployed_code.(Code.hex_name);
  |}.

  Definition initial_state : State.t :=
    State.init <| State.accounts := Constructor.final_state.(State.accounts) |>.

  Definition result_state :=
    eval_with_revert 5000 codes environment deployed_code.(Code.body) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000602222222222222222333333333333333344444444444444445555555555555555444444444444444455555555555555556666666666666666777777777777777788888888888888889999999999999999ccccccccccccccccdddddddddddddddd".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step1.

(* // mcopy_to_left_overlap() -> 0x20, 0x60, 0x2222222222222222333333333333333366666666666666667777777777777777, 0x88888888888888889999999999999999aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb, 0xaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd *)
Module Step2.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "fb6f25ef";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
    Environment.code_name := deployed_code.(Code.hex_name);
  |}.

  Definition initial_state : State.t :=
    State.init <| State.accounts := Step1.state.(State.accounts) |>.

  Definition result_state :=
    eval_with_revert 5000 codes environment deployed_code.(Code.body) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000060222222222222222233333333333333336666666666666666777777777777777788888888888888889999999999999999aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step2.

(* // mcopy_in_place() -> 0x20, 0x60, 0x2222222222222222333333333333333344444444444444445555555555555555, 0x6666666666666666777777777777777788888888888888889999999999999999, 0xaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd *)
Module Step3.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "eb48dad5";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
    Environment.code_name := deployed_code.(Code.hex_name);
  |}.

  Definition initial_state : State.t :=
    State.init <| State.accounts := Step2.state.(State.accounts) |>.

  Definition result_state :=
    eval_with_revert 5000 codes environment deployed_code.(Code.body) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000006022222222222222223333333333333333444444444444444455555555555555556666666666666666777777777777777788888888888888889999999999999999aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step3.

(* // mcopy_to_right_no_overlap() -> 0x20, 0x60, 0x2222222222222222333333333333333344444444444444445555555555555555, 0x6666666666666666777777777777777744444444444444445555555555555555, 0x66666666666666667777777777777777ccccccccccccccccdddddddddddddddd *)
Module Step4.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "50cff8ff";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
    Environment.code_name := deployed_code.(Code.hex_name);
  |}.

  Definition initial_state : State.t :=
    State.init <| State.accounts := Step3.state.(State.accounts) |>.

  Definition result_state :=
    eval_with_revert 5000 codes environment deployed_code.(Code.body) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000602222222222222222333333333333333344444444444444445555555555555555666666666666666677777777777777774444444444444444555555555555555566666666666666667777777777777777ccccccccccccccccdddddddddddddddd".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step4.

(* // mcopy_to_left_no_overlap() -> 0x20, 0x60, 0x2222222222222222333333333333333388888888888888889999999999999999, 0xaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb88888888888888889999999999999999, 0xaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd *)
Module Step5.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "a576d508";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
    Environment.code_name := deployed_code.(Code.hex_name);
  |}.

  Definition initial_state : State.t :=
    State.init <| State.accounts := Step4.state.(State.accounts) |>.

  Definition result_state :=
    eval_with_revert 5000 codes environment deployed_code.(Code.body) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000602222222222222222333333333333333388888888888888889999999999999999aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbb88888888888888889999999999999999aaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbccccccccccccccccdddddddddddddddd".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step5.
