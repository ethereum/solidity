(* Generated test file *)
Require Import CoqOfSolidity.CoqOfSolidity.
Require Import simulations.CoqOfSolidity.

Require test.libsolidity.semanticTests.types.array_mapping_abstract_constructor_param.A.
Require test.libsolidity.semanticTests.types.array_mapping_abstract_constructor_param.C.

Definition constructor_code : Code.t :=
  test.libsolidity.semanticTests.types.array_mapping_abstract_constructor_param.C.C.code.

Definition deployed_code : Code.t :=
  test.libsolidity.semanticTests.types.array_mapping_abstract_constructor_param.C.C.deployed.code.

Definition codes : list (U256.t * M.t BlockUnit.t) :=
  test.libsolidity.semanticTests.types.array_mapping_abstract_constructor_param.C.codes.

Module Constructor.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := [];
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    let address := environment.(Environment.address) in
    let account := {|
      Account.balance := environment.(Environment.callvalue);
      Account.nonce := 1;
      Account.code := constructor_code.(Code.hex_name);
      Account.codedata := Memory.hex_string_as_bytes "";
      Account.storage := Memory.init;
      Account.immutables := [];
    |} in
    Stdlib.initial_state
      <| State.accounts := [(address, account)] |>
      <| State.codes := codes |>.

  Definition result_state :=
    eval_with_revert 5000 environment constructor_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Goal Test.is_return result state = inl (Memory.u256_as_bytes deployed_code.(Code.hex_name)).
  Proof.
    vm_compute.
    reflexivity.
  Qed.

Definition final_state : State.t :=
  snd (
    eval 5000 environment (update_current_code_for_deploy deployed_code.(Code.hex_name)) state
  ).
End Constructor.

(* // m(uint256,uint256,uint256): 0, 0, 0 -> FAILURE *)
Module Step1.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "2def5e24000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Constructor.final_state.(State.accounts) |>
      <| State.codes := Constructor.final_state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "".

  Goal Test.extract_output result state Test.Status.Failure = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step1.

(* // m(uint256,uint256,uint256): 1, 0, 1 -> 2 *)
Module Step2.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "2def5e24000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step1.state.(State.accounts) |>
      <| State.codes := Step1.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000002".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step2.

(* // m(uint256,uint256,uint256): 1, 0, 5 -> 0 *)
Module Step3.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "2def5e24000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000005";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
  |}.

  Definition initial_state : State.t :=
    Stdlib.initial_state
      <| State.accounts := Step2.state.(State.accounts) |>
      <| State.codes := Step2.state.(State.codes) |>.

  Definition result_state :=
    eval_with_revert 5000 environment deployed_code.(Code.code) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step3.
