; Assembly code
; Generated automatically

; Pseudo assembly code
; Target architecture: Educational pseudo instruction set

FUNC_BEGIN main
; Unknown instruction
    STORE x, t1
    STORE y, 5.50
    LOAD temp_2, x
    LOAD temp_3, y
    CONVERT_FLOAT temp_4, t2
    MUL temp_5, t4, t3
    LOAD temp_6, x
    LOAD temp_7, y
    CONVERT_FLOAT temp_8, t6
    SUB temp_9, t8, t7
    ADD temp_10, t5, t9
    STORE result, t10
    LOAD temp_11, result
    GT temp_12, t11, 40.00
    JUMPZ t12, L1
    LOAD temp_13, "Initial result is greater than 40.0\n"
; Unknown instruction
; Unknown instruction
    JUMP L2
L1:
    LOAD temp_15, result
    MUL temp_16, t15, 2.00
    STORE result, t16
    LOAD temp_17, "Doubled result: %f\n"
; Unknown instruction
    LOAD temp_18, result
; Unknown instruction
; Unknown instruction
L2:
L3:
    LOAD temp_20, result
    LT temp_21, t20, 100.00
    JUMPZ t21, L4
    LOAD temp_22, result
    ADD temp_23, t22, 10.50
    STORE result, t23
    JUMP L3
L4:
    LOAD temp_24, "Final result: %f\n"
; Unknown instruction
    LOAD temp_25, result
; Unknown instruction
; Unknown instruction
    RETURN 0
FUNC_END

; Code generation completed
