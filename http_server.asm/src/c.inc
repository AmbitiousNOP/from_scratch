extern htons
extern inet_aton

struc sockaddr_in
.sin_family: resw 1
.sin_port: resw 1
.sin_addr: resd 1
.sin_zero: resq 1
endstruc


