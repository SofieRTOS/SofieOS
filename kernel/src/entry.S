.global _entry
_entry:
    csrr a0, mhartid
    li a1, 0
    li a2, 0
    call _start
