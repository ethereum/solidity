(* Generated test file *)
Require Import CoqOfSolidity.CoqOfSolidity.
Require Import simulations.CoqOfSolidity.

Require semanticTests.strings.concat.string_concat_different_types.C.

Definition constructor_code : Code.t :=
  semanticTests.strings.concat.string_concat_different_types.C.C.code.

Definition deployed_code : Code.t :=
  semanticTests.strings.concat.string_concat_different_types.C.C.deployed.code.

Definition codes : list Code.t :=
  semanticTests.strings.concat.string_concat_different_types.C.codes.

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

(* // f(string): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 0x25, 0x6162636461626364616263646162636461626364616263646162636461626364, 44502269928904312298000709931354278973409164155382318144318241583783949107200 *)
Module Step1.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "91e145ef000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000206162636461626364616263646162636461626364616263646162636461626364";
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
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000002561626364616263646162636461626364616263646162636461626364616263646263646566000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step1.

(* // g(string): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 0x42, 0x6162636461626364616263646162636461626364616263646162636461626364, 0x6162636465666768616263646566676861626364656667686162636465666768, 44047497324925121336511606693520958599579173549109180625971642598225011015680 *)
Module Step2.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "e6d02096000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000206162636461626364616263646162636461626364616263646162636461626364";
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
    Memory.hex_string_as_bytes "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000042616263646162636461626364616263646162636461626364616263646162636461626364656667686162636465666768616263646566676861626364656667686162000000000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step2.

(* // h(string): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 0x25, 0x6162636461626364616263646162636461626364616263646162636461626364, 44502269928904312298000709931354278973409164155382318144318241583783949107200 *)
Module Step3.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "4f744b53000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000206162636461626364616263646162636461626364616263646162636461626364";
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
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000002561626364616263646162636461626364616263646162636461626364616263646263646566000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step3.

(* // j(string): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 0x2a, 0x6162636461626364616263646162636461626364616263646162636461626364, 44502269928944786876717917111204727192787026596791669343131645116682757734400 *)
Module Step4.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "83223560000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000206162636461626364616263646162636461626364616263646162636461626364";
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
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000002a61626364616263646162636461626364616263646162636461626364616263646263646566626364656600000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step4.

(* // k(string,bytes): 0x40, 0x80, 32, "abcdabcdabcdabcdabcdabcdabcdabcd", 5, "bcdef" -> 0x20, 0x25, 0x6162636461626364616263646162636461626364616263646162636461626364, 44502269928904312298000709931354278973409164155382318144318241583783949107200 *)
Module Step5.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "3cc801f7000000000000000000000000000000000000000000000000000000000000004000000000000000000000000000000000000000000000000000000000000000800000000000000000000000000000000000000000000000000000000000000020616263646162636461626364616263646162636461626364616263646162636400000000000000000000000000000000000000000000000000000000000000056263646566000000000000000000000000000000000000000000000000000000";
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
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000002561626364616263646162636461626364616263646162636461626364616263646263646566000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step5.

(* // slice(string): 0x20, 4, "abcd" -> 0x20, 4, "abcd" *)
Module Step6.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "7198eff9000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000046162636400000000000000000000000000000000000000000000000000000000";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
    Environment.code_name := deployed_code.(Code.hex_name);
  |}.

  Definition initial_state : State.t :=
    State.init <| State.accounts := Step5.state.(State.accounts) |>.

  Definition result_state :=
    eval_with_revert 5000 codes environment deployed_code.(Code.body) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000046162636400000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step6.

(* // strParam(bytes): 0x20, 32, "abcdabcdabcdabcdabcdabcdabcdabcd" -> 0x20, 0x25, 0x6162636461626364616263646162636461626364616263646162636461626364, 44502269928904312298000709931354278973409164155382318144318241583783949107200 *)
Module Step7.
  Definition environment : Environment.t :={|
    Environment.caller := 0x1212121212121212121212121212120000000012;
    Environment.callvalue := 0;
    Environment.calldata := Memory.hex_string_as_bytes "087fa4bd000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000206162636461626364616263646162636461626364616263646162636461626364";
    Environment.address := 0xc06afe3a8444fc0004668591e8306bfb9968e79e;
    Environment.code_name := deployed_code.(Code.hex_name);
  |}.

  Definition initial_state : State.t :=
    State.init <| State.accounts := Step6.state.(State.accounts) |>.

  Definition result_state :=
    eval_with_revert 5000 codes environment deployed_code.(Code.body) initial_state.

  Definition result := fst result_state.
  Definition state := snd result_state.

  Definition expected_output : list Z :=
    Memory.hex_string_as_bytes "0000000000000000000000000000000000000000000000000000000000000020000000000000000000000000000000000000000000000000000000000000002561626364616263646162636461626364616263646162636461626364616263646263646566000000000000000000000000000000000000000000000000000000".

  Goal Test.extract_output result state Test.Status.Success = inl expected_output.
  Proof.
    vm_compute.
    reflexivity.
  Qed.
End Step7.
