; Assembly code
; Generated automatically

; Pseudo assembly code
; Target architecture: Educational pseudo instruction set

FUNC_BEGIN main
    STORE x, 10
    STORE y, 5.50
    LOAD temp_1, x
    LOAD temp_2, y
    CONVERT_FLOAT temp_3, t1
    MUL temp_4, t3, t2
    LOAD temp_5, x
    LOAD temp_6, y
    CONVERT_FLOAT temp_7, t5
    SUB temp_8, t7, t6
    ADD temp_9, t4, t8
    STORE result, t9
    LOAD temp_10, result
    GT temp_11, t10, 40.00
    JUMPZ t11, L1
    LOAD temp_12, "Initial result is greater than 40.0\n"
; Unknown instruction
; Unknown instruction
    JUMP L2
L1:
    LOAD temp_14, result
    MUL temp_15, t14, 2.00
    STORE result, t15
    LOAD temp_16, "Doubled result: %f\n"
; Unknown instruction
    LOAD temp_17, result
; Unknown instruction
; Unknown instruction
L2:
L3:
    LOAD temp_19, result
    LT temp_20, t19, 100.00
    JUMPZ t20, L4
    LOAD temp_21, result
    ADD temp_22, t21, 10.50
    STORE result, t22
    JUMP L3
L4:
    LOAD temp_23, "Final result: %f\n"
; Unknown instruction
    LOAD temp_24, result
; Unknown instruction
; Unknown instruction
    RETURN 0
FUNC_END

; Code generation completed
