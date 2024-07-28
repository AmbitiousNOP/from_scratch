%include "c.inc"

SECTION .data
ip_address: db "0.0.0.0", 0
port: dw 8085

listen_msg: db 'Listening for connections', 0Ah
.len: equ $ - listen_msg

accepted_msg: db 'Client connected', 0Ah
.len: equ $ - accepted_msg

server_addr:
	istruc sockaddr_in
	at sockaddr_in.sin_family, dw 2
	at sockaddr_in.sin_port, dw 0
	at sockaddr_in.sin_addr, dd 0
	at sockaddr_in.sin_zero, dq 0
	iend

client_addr:
	istruc sockaddr_in
	at sockaddr_in.sin_family, dw 2
	at sockaddr_in.sin_port, dw 0
	at sockaddr_in.sin_addr, dd 0
	at sockaddr_in.sin_zero, dq 0
	iend
client_addr_size:
	dd 16

SECTION .bss
;port: resq 1	; TODO: delete port declaration in .data section and get from argv
server_socket resq 1
client_socket resq 1

SECTION .text
	global main

main: 
	; get a socket file descriptor
	mov rax, 41	; syscall socket()
	mov rdi, 2	; AF_INET
	mov rsi, 1	; TCP	
	mov rdx, 0	; defaults to 0 but just incase rdx isnt inti'd to 0
	syscall 
	cmp rax, 0			; check for error returned by socket syscall
	jl close_server			; if < 0 close_server due to error.
	mov [server_socket], rax	; move the file descriptor to server_socket

	; convert port to nework byte order
	;NOTE: bind() freaks out when i pass in manually.
	;TODO: manually pass it and take out htons 
	mov rdi, [port]
	call htons
	mov [server_addr+sockaddr_in.sin_port], rax	; move returned value into struct 
	; FIX: add error handling.

	; convert ip_address to network byte order
	;NOTE: bind() again freaks out when i pass in manually.
	;TODO: manually pass it and take out inet_aton.
	mov rdi, ip_address
	mov rsi, server_addr + sockaddr_in.sin_addr
	call inet_aton
	;FIX: add error handling. 

	; setting the socket options to re-use address
	mov rax, 54			; syscall setsockopt()
	mov rdi, [server_socket]	; file descriptor
	mov rsi, 1			; SOL_SOCKET
	mov rdx, 2			; SO_REUSEADDR
	mov r10, 1			; reuseaddr_enabled
	mov r8, 4
	syscall 
	cmp rax, 0			; check for error 
	jge close_server		; if error close server

	; assign address to socket 	
	mov rax, 49			; syscall bind()
	mov rdi, [server_socket]	; file descriptor
	mov rsi, server_addr		; pointer to struct 
	mov rdx, 16			; length of struct
	syscall 
	cmp rax, 0			; error checking
	jne close_server		; if not 0 close server
	
	; listen for incoming clients
	mov rax, 50			; syscall for listen
	mov rdi, [server_socket]	; file descriptor 
	mov rsi, 5			; the number of clinets that can queue for connections
	syscall 
	cmp rax, 0
	jne close_server

	xor r8, 0			; make r8 0 for loop
	cmp r8, 1			; check for exit from loop
	jne .server_accept_loop		; jump to server loop


	;TODO: recv() data from the client
	;TODO: send() resp to client
	;TODO: close() client_fd connection

	; close client fd
	mov rax, 3
	mov rdi, [client_addr]
	syscall 

	; close server fd
	mov rax, 3			; syscall for close()
	mov rdi, [server_socket]	; file descriptor to close
	syscall

	; exit program
	mov rax, 60			; syscall for exit()
	xor rdi, rdi			; return 0 for success
	syscall	

.server_accept_loop:
	;FIX: handle SIGINTS

	; https://man7.org/linux/man-pages/man2/signalfd.2.html
	; print out listening message 
	mov rax, 1	; write() syscall
	mov rdi, 1
	mov rsi, listen_msg
	mov rdx, listen_msg.len
	syscall 
	cmp rax, listen_msg.len		; num of bytes written compared to length of msg.
	; TODO: create func to jump to if rax is shorter then msg length

	; accept incoming connections from clients
	mov rax, 43			; syscall for accept
	mov rdi, [server_socket]	; file descriptor 
	mov rsi, client_addr		; address of client struct to be filled by accept()
	mov rdx, client_addr_size			; size of server struct
	syscall 
	cmp rax, 0
	jl close_server
	mov [client_socket], rax	; store the clients file descriptor in client_server 

	; print out accepted connection.
	mov rax, 1
	mov rdi, 1
	mov rsi, accepted_msg
	mov rdx, accepted_msg.len
	syscall
	cmp rax, accepted_msg.len
	; TODO: create func to jump to if rax is shorter then msg legnth 

	cmp r8, 1
	jne .server_accept_loop
	ret 

close_server:
	; close fd
	mov rax, 3			; syscall for close()
	mov rdi, [server_socket]	; fiel descriptor to close
	syscall 

	; exit program 
	mov rax, 60			; syscall for exit()
	xor rdi, 1			; exit with error (1) 
	syscall


