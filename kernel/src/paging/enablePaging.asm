

enablePaging:
  ; move page table address to cr3
  mov rax, rdi
  mov cr3, rax

  ; enable PAE
  mov rax, cr4
  or rax, 1 << 5
  mov cr4, rax



  ; enable paging
  mov rax, cr0
  or rax, 1 << 31
  or rax, 1 << 16
  mov cr0, rax

  retq

GLOBAL enablePaging